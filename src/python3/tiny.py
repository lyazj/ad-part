import torch
import logging

# base class for Tiny*
class TinyModule(torch.nn.Module):

    forward_depth = 0

    def __init__(self, name=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.name = name or self.__class__.__name__  # diagnostic

    def begin_forward(self):
        TinyModule.forward_depth += 1
        logging.info(f'enter [{TinyModule.forward_depth}]: {self.name}')

    def end_forward(self):
        logging.info(f'leave [{TinyModule.forward_depth}]: {self.name}')
        TinyModule.forward_depth -= 1

# flow structure wrapper 1/2: sequential
class TinySequential(TinyModule):

    def __init__(self, layers, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.layers = layers

    def forward(self, *args, **kwargs):
        super().begin_forward()
        try:
            for layer in self.layers:
                if isinstance(args, tuple): args = layer(*args, **kwargs)
                else: args = layer(args, **kwargs)
        finally:
            super().end_forward()
        return args

# flow structure wrapper 2/2: residual
class TinyResidual(TinyModule):

    # requires: x.shape == module(x).shape
    def __init__(self, module, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.module = module

    def forward(self, x, *args, **kwargs):
        super().begin_forward()
        try:
            args = self.module(x, *args, **kwargs)
            if isinstance(args, tuple): args = (args[0] + x, *args[1:])
            else: args = args + x
        finally:
            super().end_forward()
        return args

# multi-layer preceptor
class TinyMLP(TinySequential):

    # (batch_size, input_dim) -> Linear(dims[0]) [ -> activate ] -> ... -> (batch_size, output_dim)
    # output_dim = dims[-1] if len(dims) else input_dim
    def __init__(self, input_dim, dims, bias=True, activate=torch.nn.ReLU,
                 no_post_activation=True, *args, **kwargs):
        layers = []
        for i, dim in enumerate(dims, 1):
            layers.append(torch.nn.Linear(input_dim, dim, bias))
            if activate is not None and ((not no_post_activation) or i != len(dims)): layers.append(activate())
            input_dim = dim
        super().__init__(layers, *args, **kwargs)

# multi-head attention
class TinyMHA(TinyModule):

    # (batch_size, input_dim) -> MHA -> (batch_size, input_dim)
    def __init__(self, *args, **kwargs):
        super().__init__()
        self.attn = torch.nn.MultiheadAttention(*args, **kwargs, batch_first=True)

    def forward(self, x, *args, **kwargs):
        super().begin_forward()
        try:
            x, _ = self.attn(x, x, x, *args, need_weights=False, **kwargs)
        finally:
            super().end_forward()
        return x

# transformer block
class TinyBlock(TinySequential):

    # (batch_size, input_dim) -> LN -> resMHA -> LN -> resMLP [ -> DROP ] -> (batch_size, output_dim)
    # output_dim = dims[-1] if len(dims) else input_dim
    def __init__(self, input_dim, mlp_dims, num_heads=4, attn_dropout=0.1, attn_bias=True,
                 mlp_dropout=0.1, mlp_bias=True, mlp_activate=torch.nn.ReLU, *args, **kwargs):
        if len(mlp_dims) and mlp_dims[-1] == -1: mlp_dims = (*mlp_dims[:-1], input_dim)
        if len(mlp_dims) and mlp_dims[-1] != input_dim: raise ValueError(f'expect {input_dims} nodes as MLP output')
        layers = []
        layers.append(torch.nn.LayerNorm(input_dim))
        layers.append(TinyResidual(TinyMHA(input_dim, num_heads, attn_dropout, attn_bias)))
        layers.append(torch.nn.LayerNorm(input_dim))
        layers.append(TinyResidual(TinyMLP(input_dim, mlp_dims, mlp_bias, mlp_activate)))
        if mlp_dropout != 0.0: layers.append(torch.nn.Dropout(mlp_dropout))
        super().__init__(layers, *args, **kwargs)

# transformer
class TinyTransformer(TinySequential):

    # (batch_size, input_dim) -> MLP -> BLOCK [ -> BLOCK ] -> MLP [ -> ACTIVATE ] -> (batch_size, output_dim)
    # output_dim = decode_dims[-1] if len(decode_dims) else embed_dims[-1] if len(embed_dims) else input_dim
    def __init__(self, input_dim, embed_dims, decode_dims, num_blocks=2, activate=None,
                 embed_activate=torch.nn.ReLU, decode_activate=torch.nn.ReLU, *args, **kwargs):
        layers = []
        layers.append(TinyMLP(input_dim, embed_dims, activate=embed_activate))
        if len(embed_dims): input_dim = embed_dims[-1]
        for _ in range(num_blocks):
            layers.append(TinyBlock(input_dim, *args, **kwargs))
        layers.append(TinyMLP(input_dim, decode_dims, activate=decode_activate))
        if activate: layers.append(activate())
        super().__init__(layers)

# classifier
class TinyClassifier(TinyTransformer):

    def __init__(self, input_dim, num_classes, embed_dims=(64, 32), decode_dims=(32, -1),
                 activate=torch.nn.Softmax, mlp_dims=(64, -1), *args, **kwargs):
        if len(decode_dims) and decode_dims[-1] == -1: decode_dims = (*decode_dims[:-1], num_classes)
        if len(decode_dims) and decode_dims[-1] != num_classes: raise ValueError(f'expect {num_classes} nodes at the end')
        super().__init__(input_dim, embed_dims, decode_dims, activate=activate, mlp_dims=mlp_dims, *args, **kwargs)

# binary classifier
class TinyBinaryClassifier(TinyClassifier):

    def __init__(self, input_dim, activate=torch.nn.Sigmoid, *args, **kwargs):
        super().__init__(input_dim, 1, activate=activate, *args, **kwargs)
