#!/usr/bin/env python3

import os
import re
import uproot
import builtins
import numpy as np
import awkward as ak
import mplhep as hep
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve
import pyhf
from pyhf.contrib.viz import brazil

# Transcript printed content to a same-name log file.
try: os.remove(re.sub(r'\.py$', '.log', __file__))
except Exception: pass
logfile = open(re.sub(r'\.py$', '.log', __file__), 'w')
def print(*args, **kwargs): builtins.print(*args, **kwargs); builtins.print(*args, **{**kwargs, 'file': logfile})

plt.style.use(hep.style.CMS)

SIGNAL = 'gghh'
POSTFIX = re.search(r'(?:^|/)limit([^/]*)\.py$', __file__).group(1) or '_default'
NEVENT_MAX = None
#NEVENT_MAX = 10000

event_expressions = None
labels = {
    'QCD':   r'QCD',
    'VJets': r'V + Jets',
    'TTbar': r'TTbar',
    'gghh':  r'$ggHH\ (HH \to 4b)$',
}
weights_gghh = {
    'ggHH_kl_0_kt_1': lambda kl, kt: (0.2*kl**2 - 1.2*kl + 1.0) * 2.508E-04 * 43834,
    'ggHH_kl_1_kt_1': lambda kl, kt: (-0.25*kl**2 + 1.25*kl) * 1.172E-04 * 73366,
    'ggHH_kl_5_kt_1': lambda kl, kt: (0.05*kl**2 - 0.05*kl) * 3.154E-04 * 34757,
}
weights = {
    'QCD':   51400000,
    'VJets': 571000 + 128000 + 225000 + 25800 + 22600,
    'TTbar': 246000,
}
categories = labels.keys()

# Compute expressions to be evaluated on input ROOT files.
if event_expressions is not None:
    expressions = [expression[1] for expression in event_expressions]
    print('Expressions:', *expressions, sep='\n  - ')
else:
    expressions = None
    print('Expressions:', expressions)

# Evaluate expressions on specified input ROOT files. Read at most n events.
def concatenate(files, expressions, n=None):
    events = []
    for file in files:
        if n is not None and n == 0: break
        with uproot.open(file) as file:
            events.append(file.arrays(expressions=expressions, entry_start=0, entry_stop=n))
        if n is not None: n -= len(events)
    events = ak.concatenate(events)
    if event_expressions is None: return events
    aliased_events = ak.Array({ alias: events[origin] for alias, origin in event_expressions })
    return aliased_events

prediction = { }

for method in ['raw', 'lite', 'full', 'hid']:
    print('\n' + method, end='\n' + '-' * 40 + '\n')
    events = { }
    prediction[method] = events

    rootfile = '../../run/2H4B-%s/predict_output/pred%s.root' % (method, POSTFIX)
    rootfiles = [rootfile + ':Events']
    print('ROOT files:', *rootfiles, sep='\n  - ')
    print('Loading events...')
    uncategorized_events = concatenate(rootfiles, expressions, NEVENT_MAX)
    print('%d events loaded.' % len(uncategorized_events))
    uncategorized_events['2H4BVSQCD'] = 1.0 / (1.0 + uncategorized_events['score_label_QCD'] / uncategorized_events['score_label_2H4B'])

    for category in weights:
        events[category] = uncategorized_events[uncategorized_events['label_' + category] == True]
        events[category]['weight'] = weights[category] / len(events[category])

    for category in weights_gghh:
        rootfiles = ['../../run/gghh-%s/pred%s_%%s.root' % (method, POSTFIX) % category + ':Events']
        print('ROOT files:', *rootfiles, sep='\n  - ')
        print('Loading events...')
        events[category] = concatenate(rootfiles, expressions, NEVENT_MAX)
        print('%d events loaded.' % len(events[category]))
        events[category]['2H4BVSQCD'] = 1.0 / (1.0 + events[category]['score_label_QCD'] / events[category]['score_label_2H4B'])

events = { }
prediction['none'] = events
for category in prediction['lite']:
    events[category] = ak.copy(prediction['lite'][category])
    #events[category]['2H4BVSQCD'] = 1.0 / (1.0 + (events[category]['lead_jet_probQCD'] * events[category]['sublead_jet_probQCD']) / (events[category]['lead_jet_probHbb'] * events[category]['sublead_jet_probHbb']))
    lead_jet_HbbVSQCD = 1.0 / (1.0 + events[category]['lead_jet_probQCD'] / events[category]['lead_jet_probHbb'])
    sublead_jet_HbbVSQCD = 1.0 / (1.0 + events[category]['sublead_jet_probQCD'] / events[category]['sublead_jet_probHbb'])
    events[category]['2H4BVSQCD'] = np.minimum(lead_jet_HbbVSQCD, sublead_jet_HbbVSQCD)

raw_prediction = prediction
del prediction

def reweight(raw_events, kl, kt):
    events = { }
    events_gghh = [ ]
    for category in raw_events:
        if category.startswith('ggHH'):
            import copy
            e = copy.deepcopy(raw_events[category])
            e['weight'] = weights_gghh[category](kl, kt) / len(e)
            events_gghh.append(e)
        else:
            events[category] = raw_events[category]
    events['gghh'] = ak.concatenate(events_gghh)
    return events

def reweight_prediction(kl, kt):
    return { c: reweight(e, kl, kt) for (c, e) in raw_prediction.items() }

os.makedirs('limit' + POSTFIX, exist_ok=True)
try: os.remove(os.path.join('limit' + POSTFIX, re.sub(r'\.py$', '.log', __file__)))
except Exception: pass
os.link(re.sub(r'\.py$', '.log', __file__),
        os.path.join('limit' + POSTFIX, re.sub(r'\.py$', '.log', __file__)))

def get_signif(s, b):
    return s / np.sqrt(b + 1)

def apply_mass_window(events):
    events = events[events['lead_jet_sdmass'] >= 100]
    events = events[events['lead_jet_sdmass'] <= 150]
    return events

for events in raw_prediction.values():
    for category in events:
        events[category] = apply_mass_window(events[category])

def make_best_cut(events):
    signal_events = events[SIGNAL]
    background_events = ak.concatenate([events[category] for category in events if category != SIGNAL])
    y_true = np.concatenate([np.ones(len(signal_events)), np.zeros(len(background_events))])
    y_score = np.concatenate([signal_events['2H4BVSQCD'], background_events['2H4BVSQCD']])
    sample_weight = np.concatenate([signal_events['weight'], background_events['weight']])
    #print('Generating ROC curve...')
    fpr, tpr, thr = roc_curve(y_true, y_score, sample_weight=sample_weight)
    i = len(fpr) - 1 - np.argmax(fpr[::-1] < 10**-5.5)
    fpr, tpr, thr = fpr[i:], tpr[i:], thr[i:]
    s_org = np.sum(signal_events['weight'])
    b_org = np.sum(background_events['weight'])
    s, b = s_org * tpr, b_org * fpr
    signif = get_signif(s, b)
    i = signif.argmax()
    #print('best: thr=%.4f s=%.3f b=%.3f signif=%.5f' % (thr[i], s[i], b[i], signif[i]))
    signal_events = signal_events[signal_events['2H4BVSQCD'] >= thr[i]]
    background_events = background_events[background_events['2H4BVSQCD'] >= thr[i]]
    return signal_events, background_events

def get_exp_limits(method, raw_events, kl, kt):
    events = reweight(raw_events, kl, kt)
    signal_events, background_events = make_best_cut(events)
    signal = np.histogram(np.array(signal_events['lead_jet_sdmass']), 5, (100, 150), weights=np.array(signal_events['weight']))[0]
    background = np.histogram(np.array(background_events['lead_jet_sdmass']), 5, (100, 150), weights=np.array(background_events['weight']))[0]
    background_unc = np.sqrt(background)  # [FIXME]
    model = pyhf.simplemodels.uncorrelated_background(signal=signal, bkg=background, bkg_uncertainty=background_unc)
    observations = model.expected_data(model.config.suggested_init())  # [XXX]
    bounds = model.config.suggested_bounds()
    bounds[model.config.poi_index] = (0.0, 1e10)
    scan = None
    obs_limit, exp_limits, (scan, results) = pyhf.infer.intervals.upper_limits.upper_limit(
        observations, model, scan, level=0.05, return_results=True, par_bounds=bounds
    )
    print(method, kl, exp_limits, sep='\t')
    return exp_limits

def get_exp_limits_monoarg(args):
    return get_exp_limits(*args)

from multiprocessing import Pool
pool = Pool(32)
kt = 1.0
kl_array = np.linspace(-10, 20, 31)
methods = ['none', 'raw', 'lite', 'full', 'hid']
raw_events_list = [ raw_prediction[method] for method in methods ]
args_list = [ ]
for method in methods:
    for kl in kl_array:
        args_list.append((method, raw_prediction[method], kl, kt))
limits = np.array(pool.map(get_exp_limits_monoarg, args_list)).reshape(len(methods), len(kl_array), -1)
np.save(os.path.join('limit' + POSTFIX, 'limits.npy'), limits)
