import os
import sys

basedir = os.path.abspath(__file__)
while basedir != '/' and os.path.basename(basedir) != 'ad-part':
    basedir = os.path.dirname(basedir)
sys.path.insert(0, os.path.join(basedir, 'src', 'python3'))

import adjet
import tiny
import torch

class Model(torch.nn.Module):

    def __init__(self):
        super().__init__()
        self.model = tiny.TinyEventClassifier(adjet.EVT_LABEL, adjet.JET_LABEL, adjet.NFEAT_LEP, adjet.NFEAT_PHO,
                                              classifier=tiny.TinyClassifier, num_classes=5, num_blocks=2)
        self.to(tiny.device)

    def forward(self, evt, jet, lep, pho):
        evt = evt.permute(0, 2, 1).contiguous().to(tiny.device)
        jet = jet.permute(0, 2, 1).contiguous().to(tiny.device)
        lep = lep.permute(0, 2, 1).contiguous().to(tiny.device)
        pho = pho.permute(0, 2, 1).contiguous().to(tiny.device)
        evt_msk = torch.zeros(evt.shape[:2], dtype=torch.bool).to(tiny.device)
        jet_msk = torch.arange(jet.shape[1]).unsqueeze(0).to(tiny.device) >= evt[:,0,adjet.EVT_NJET].unsqueeze(1)
        lep_msk = torch.arange(lep.shape[1]).unsqueeze(0).to(tiny.device) >= evt[:,0,adjet.EVT_NLEP].unsqueeze(1)
        pho_msk = torch.arange(pho.shape[1]).unsqueeze(0).to(tiny.device) >= evt[:,0,adjet.EVT_NPHO].unsqueeze(1)
        msk = torch.concat([evt_msk, jet_msk, lep_msk, pho_msk], dim=1)
        self.model.to(tiny.device)  # [XXX]
        x, msk = self.model(evt, jet, lep, pho, msk)
        return x

def get_model(data_config, **kwargs):
    model = Model()
    input_shapes = data_config.input_shapes
    input_shapes = {k: (1, *v[1:]) for k, v in input_shapes.items()}
    input_shapes['msk'] = (1, 31)
    dynamic_axes = {
        'evt': {0: 'N'},
        'jet': {0: 'N', 2: 'evt_njet'},
        'lep': {0: 'N', 2: 'evt_nlep'},
        'pho': {0: 'N', 2: 'evt_npho'},
    }
    model_info = {
        'input_names': data_config.input_names,
        'input_shapes': input_shapes,
        'output_names': ['output'],
        'dynamic_axes': dynamic_axes,
    }
    return model, model_info

def get_loss(data_config, **kwargs):
    return torch.nn.CrossEntropyLoss()
