#!/usr/bin/env python3

import os
import re
import uproot
import glob
import builtins
import numpy as np
import awkward as ak
import mplhep as hep
import matplotlib.pyplot as plt
from sklearn.metrics import roc_curve

# Transcript printed content to a same-name log file.
logfile = open(re.sub(r'\.py$', '.log', __file__), 'w')
def print(*args, **kwargs): builtins.print(*args, **kwargs); builtins.print(*args, **{**kwargs, 'file': logfile})

plt.style.use(hep.style.CMS)

SIGNAL = 'HH4B'
POSTFIX = re.search(r'(?:^|/)plot([^/]*)\.py$', __file__).group(1) or '_default'
os.makedirs('plot' + POSTFIX, exist_ok=True)
NEVENT_MAX = None
#NEVENT_MAX = 1000000

event_expressions = None
categories = ['QCD', 'VJets', 'TTbar', 'HH4B']
labels = {
    'QCD':   r'QCD',
    'VJets': r'V + Jets',
    'TTbar': r'TTbar',
    'HH4B':  r'$2H \to 4b$',
}
weights = {
    'QCD':   51400000,
    'VJets': 571000 + 128000 + 225000 + 25800 + 22600,
    'TTbar': 246000,
    'HH4B':  1630 * (37.9 / 48.5e3) * 0.582**2,
}

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
    uncategorized_events = uncategorized_events[uncategorized_events['lead_jet_sdmass'] >= 100]
    uncategorized_events = uncategorized_events[uncategorized_events['lead_jet_sdmass'] <= 150]
    uncategorized_events = uncategorized_events[uncategorized_events['sublead_jet_sdmass'] >= 100]
    uncategorized_events = uncategorized_events[uncategorized_events['sublead_jet_sdmass'] <= 150]
    print('%d events in mass window [100, 150].' % len(uncategorized_events))
    uncategorized_events['2H4BVSQCD'] = 1.0 / (1.0 + uncategorized_events['score_label_QCD'] / uncategorized_events['score_label_2H4B'])

    for category in categories:
        events[category] = uncategorized_events[uncategorized_events['label_' + category.replace('HH4B', '2H4B')] == True]
        events[category]['weight'] = weights[category] / len(events[category])

events = { }
prediction['none'] = events
for category in categories:
    events[category] = ak.copy(prediction['lite'][category])
    #events[category]['2H4BVSQCD'] = 1.0 / (1.0 + (events[category]['lead_jet_probQCD'] * events[category]['sublead_jet_probQCD']) / (events[category]['lead_jet_probHbb'] * events[category]['sublead_jet_probHbb']))
    lead_jet_HbbVSQCD = 1.0 / (1.0 + events[category]['lead_jet_probQCD'] / events[category]['lead_jet_probHbb'])
    sublead_jet_HbbVSQCD = 1.0 / (1.0 + events[category]['sublead_jet_probQCD'] / events[category]['sublead_jet_probHbb'])
    events[category]['2H4BVSQCD'] = np.minimum(lead_jet_HbbVSQCD, sublead_jet_HbbVSQCD)

def savefig(path, *args, **kwargs):
    print('Saving to %s...' % path)
    plt.savefig(os.path.join('plot' + POSTFIX, path), *args, **kwargs)

def get_signif(s, b):
    return s / np.sqrt(b + 1)

def roc(events, *args, **kwargs):
    signal_events = events[SIGNAL]
    background_events = ak.concatenate([events[category] for category in events if category != SIGNAL])
    y_true = np.concatenate([np.ones(len(signal_events)), np.zeros(len(background_events))])
    y_score = np.concatenate([signal_events['2H4BVSQCD'], background_events['2H4BVSQCD']])
    sample_weight = np.concatenate([signal_events['weight'], background_events['weight']])
    print('Generating ROC curve...')
    fpr, tpr, _ = roc_curve(y_true, y_score, sample_weight=sample_weight)
    i = len(fpr) - 1 - np.argmax(fpr[::-1] < 10**-4.5)
    fpr, tpr = fpr[i:], tpr[i::]
    s_org = np.sum(signal_events['weight'])
    b_org = np.sum(background_events['weight'])
    s, b = s_org * tpr, b_org * fpr
    signif = get_signif(s, b)
    return plt.plot(fpr, signif, *args, **kwargs)

plt.figure(figsize=(12, 9), dpi=150)
roc(prediction['none'], label='none')
roc(prediction['raw'], label='raw')
roc(prediction['lite'], label='lite')
roc(prediction['full'], label='full')
roc(prediction['hid'], label='hid')
plt.xlabel(r'Background suppression'); plt.ylabel('Significance')
plt.xscale('log')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('roc.pdf')
plt.close()
