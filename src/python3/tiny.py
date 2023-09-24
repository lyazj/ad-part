import torch
import logging
import functools

device = 'cuda:0'

# base class for Tiny*
class TinyModule(torch.nn.Module):

    forward_depth = 0

    def __init__(self, name=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.to(device)
        self.name = name or self.__class__.__name__  # diagnostic

    def begin_forward(self, *args, **kwargs):
        TinyModule.forward_depth += 1
        logging.debug(f'enter [{TinyModule.forward_depth}]: {self.name}')
        #args = '\n'.join(map(str, args))
        #logging.debug(f'args:\n{args}\nkwargs: {kwargs}')

    def end_forward(self):
        logging.debug(f'leave [{TinyModule.forward_depth}]: {self.name}')
        TinyModule.forward_depth -= 1

# flow structure wrapper 1/2: sequential
class TinySequential(TinyModule):

    def __init__(self, layers, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.layers = layers
        # Ensure that self.parameters() sees all parameters.
        params = []
        for layer in layers:
            if isinstance(layer, tuple): layer = layer[0]
            params.append(layer)
        self.params = torch.nn.ParameterList(params)

    def forward(self, *args, **kwargs):
        super().begin_forward(*args, **kwargs)
        try:
            # The implementation allows the last half of arguments to bypass some modules.
            # Syntax 1/2: layer ::= module
            # Syntax 2/2: layer ::= (module, num_args_pass)
            for layer in self.layers:
                if isinstance(layer, tuple): module, num_args_pass = layer
                else: module, num_args_pass = layer, len(args)
                args_pass, args_bypass = args[:num_args_pass], args[num_args_pass:]
                args_pass = module(*args_pass, **kwargs)
                args_pass = args_pass if isinstance(args_pass, tuple) else (args_pass,) if args_pass is not None else ()
                args = (*args_pass, *args_bypass)
        finally:
            super().end_forward()
        if len(args) == 1: args = args[0]
        return args

# flow structure wrapper 2/2: residual
class TinyResidual(TinyModule):

    # requires: x.shape == module(x).shape
    def __init__(self, module, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.module = module

    def forward(self, x, *args, **kwargs):
        super().begin_forward(x, *args, **kwargs)
        try:
            args = self.module(x, *args, **kwargs)
            if isinstance(args, tuple): args = (args[0] + x, *args[1:])
            else: args = args + x
        finally:
            super().end_forward()
        return args

# multi-layer perceptron
class TinyMLP(TinySequential):

    # (N, input_dim) [ -> Linear(dims[0]) [ -> activate ] -> ... ] -> (N, output_dim)
    # (N, C, input_dim) [ -> ...] -> (N, C, output_dim)
    # output_dim = dims[-1] if len(dims) else input_dim
    def __init__(self, input_dim, dims, bias=True, activate=torch.nn.ReLU,
                 no_post_activation=True, *args, **kwargs):
        layers = []
        for i, dim in enumerate(dims, 1):
            layers.append(torch.nn.Linear(input_dim, dim, bias).to(device))
            if activate is not None and ((not no_post_activation) or i != len(dims)): layers.append(activate().to(device))
            input_dim = dim
        super().__init__(layers, *args, **kwargs)

# multi-head attention
class TinyMHA(TinyModule):

    # (N, input_dim) -> MHA -> (N, input_dim)
    # (N, C, input_dim) -> MHA -> (N, C, input_dim)
    def __init__(self, *args, **kwargs):
        super().__init__()
        self.attn = torch.nn.MultiheadAttention(*args, **kwargs, batch_first=True).to(device)

    def forward(self, x, *args, **kwargs):
        super().begin_forward(x, *args, **kwargs)
        try:
            x, _ = self.attn(x, x, x, *args, need_weights=False, **kwargs)
        finally:
            super().end_forward()
        return x

# transformer block
class TinyBlock(TinySequential):

    # (N, input_dim) -> LN -> resMHA -> LN -> resMLP [ -> DROP ] -> (N, output_dim)
    # (N, C, input_dim) -> LN -> resMHA -> LN -> resMLP [ -> DROP ] -> (N, C, output_dim)
    # output_dim = dims[-1] if len(dims) else input_dim
    def __init__(self, input_dim, mlp_dims, num_heads=4, attn_dropout=0.1, attn_bias=True,
                 mlp_dropout=0.1, mlp_bias=True, mlp_activate=torch.nn.ReLU, *args, **kwargs):
        if len(mlp_dims) and mlp_dims[-1] == -1: mlp_dims = (*mlp_dims[:-1], input_dim)
        if len(mlp_dims) and mlp_dims[-1] != input_dim: raise ValueError(f'expect {input_dims} nodes as MLP output')
        layers = []
        layers.append((torch.nn.LayerNorm(input_dim).to(device), 1))
        layers.append(TinyResidual(TinyMHA(input_dim, num_heads, attn_dropout, attn_bias)))
        layers.append((torch.nn.LayerNorm(input_dim).to(device), 1))
        layers.append((TinyResidual(TinyMLP(input_dim, mlp_dims, mlp_bias, mlp_activate)), 1))
        if mlp_dropout != 0.0: layers.append((torch.nn.Dropout(mlp_dropout).to(device), 1))
        super().__init__(layers, *args, **kwargs)

# pooling
class TinyPool(TinyModule):

    def __init__(self, func=torch.max, dim=-1, keepdim=False, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.func = functools.partial(func, dim=dim, keepdim=keepdim)

    def forward(self, *args, **kwargs):
        args = self.func(*args, **kwargs)
        if issubclass(args.__class__, tuple): args = args[0]
        return args

# transformer
class TinyTransformer(TinySequential):

    # (N, input_dim) -> MLP -> BLOCK [ -> BLOCK ] -> MLP [ -> ACTIVATE ] -> (N, output_dim)
    # (N, C, input_dim) -> MLP -> BLOCK [ -> BLOCK ] -> POOL -> MLP [ -> ACTIVATE ] -> (N, output_dim)
    # output_dim = decode_dims[-1] if len(decode_dims) else embed_dims[-1] if len(embed_dims) else input_dim
    def __init__(self, dim, input_dim, embed_dims, decode_dims, num_blocks=2, activate=None,
                 embed_activate=torch.nn.ReLU, decode_activate=torch.nn.ReLU, pool=torch.max, *args, **kwargs):
        layers = []
        layers.append((TinyMLP(input_dim, embed_dims, activate=embed_activate), 1))
        if len(embed_dims): input_dim = embed_dims[-1]
        for _ in range(num_blocks):
            layers.append(TinyBlock(input_dim, *args, **kwargs))
        if dim == 1: pass
        elif dim == 2: layers.append((TinyPool(func=pool, dim=-2), 1))
        else: raise ValueError(f'invalid dimension: {dim}')
        layers.append((TinyMLP(input_dim, decode_dims, activate=decode_activate), 1))
        if activate: layers.append((activate().to(device), 1))
        super().__init__(layers)

# classifier
class TinyClassifier(TinyTransformer):

    def __init__(self, dim, input_dim, num_classes, embed_dims=(64, 32), decode_dims=(32, -1),
                 activate=torch.nn.Softmax, mlp_dims=(64, -1), *args, **kwargs):
        if len(decode_dims) and decode_dims[-1] == -1: decode_dims = (*decode_dims[:-1], num_classes)
        if len(decode_dims) and decode_dims[-1] != num_classes: raise ValueError(f'expect {num_classes} nodes at the end')
        super().__init__(dim, input_dim, embed_dims, decode_dims, activate=activate, mlp_dims=mlp_dims, *args, **kwargs)

    def run(self, data_input, data_label, loss_func=torch.nn.functional.cross_entropy):
        data_output = self(data_input)
        loss = loss_func(data_output, data_label)
        data_output_top1 = data_output.argmax(-1, True)
        data_output_acc = (data_output_top1 == data_label).to(data_output.dtype)
        return data_output, loss, data_output_top1, data_output_acc

# binary classifier
class TinyBinaryClassifier(TinyClassifier):

    def __init__(self, dim, input_dim, activate=torch.nn.Sigmoid, *args, **kwargs):
        super().__init__(dim, input_dim, 1, activate=activate, *args, **kwargs)

    def run(self, data_input, data_label, loss_func=torch.nn.functional.binary_cross_entropy):
        data_output = self(data_input)
        loss = loss_func(data_output, data_label)
        data_output_top1 = data_output > 0.5
        data_output_acc = (data_output_top1 == data_label).to(data_output.dtype)
        return data_output, loss, data_output_top1, data_output_acc

# event classifier
class TinyEventClassifier(TinyModule):

    def __init__(self, evt_dim, jet_dim, lep_dim, classifier=TinyBinaryClassifier, embed_dims=(),
                 evt_embed=(64, 32), jet_embed=(64, 32), lep_embed=(64, 32), *args, **kwargs):
        evt_embed_dim = evt_embed[-1] if evt_embed else evt_dim
        jet_embed_dim = jet_embed[-1] if jet_embed else jet_dim
        lep_embed_dim = lep_embed[-1] if lep_embed else lep_dim
        if not evt_embed_dim == jet_embed_dim == lep_embed_dim:
            raise ValueError('inconsistent dimensions after embedding')
        super().__init__()
        self.evt_embed = TinyMLP(evt_dim, evt_embed)
        self.jet_embed = TinyMLP(jet_dim, jet_embed)
        self.lep_embed = TinyMLP(lep_dim, lep_embed)
        self.classifier = classifier(2, evt_embed_dim, embed_dims=embed_dims, *args, **kwargs)

    def embed(self, evt, jet, lep, *args, **kwargs):
        evt = self.evt_embed(evt, *args, **kwargs)
        jet = self.jet_embed(jet, *args, **kwargs)
        lep = self.lep_embed(lep, *args, **kwargs)
        return torch.concat((evt, jet, lep), dim=-2)

    def forward(self, evt, jet, lep, *args, **kwargs):
        super().begin_forward(evt, jet, lep, *args, **kwargs)
        try:
            return self.classifier(self.embed(evt, jet, lep), *args, **kwargs)
        finally:
            super().end_forward()

    def run(self, evt, jet, lep, label, *args, **kwargs):
        return self.classifier.run(self.embed(evt, jet, lep), label, *args, **kwargs)
