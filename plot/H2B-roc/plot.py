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

# Transcript printed content to a same-name log file.
logfile = open(re.sub(r'\.py$', '.log', __file__), 'w')
def print(*args, **kwargs): builtins.print(*args, **kwargs); builtins.print(*args, **{**kwargs, 'file': logfile})

plt.style.use(hep.style.CMS)

SIGNAL = 'H2B'
POSTFIX = re.search(r'(?:^|/)plot([^/]*)\.py$', __file__).group(1) or '_default'
os.makedirs('plot' + POSTFIX, exist_ok=True)
NEVENT_MAX = None
#NEVENT_MAX = 1000000

event_expressions = None
categories = ['QCD', 'VJets', 'TTbar', 'H2B']
labels = {
    'QCD':   r'QCD',
    'VJets': r'V + Jets',
    'TTbar': r'TTbar',
    'H2B':   r'$H \to 2b$',
}
weights = {
    'QCD':   51400000,
    'VJets': 571000 + 128000 + 225000 + 25800 + 22600,
    'TTbar': 246000,
    'H2B':   48.5 * 0.582 * 100,
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

    rootfile = '../../run/H2B-%s/predict_output/pred%s.root' % (method, POSTFIX)
    rootfiles = [rootfile + ':Events']
    print('ROOT files:', *rootfiles, sep='\n  - ')
    print('Loading events...')
    uncategorized_events = concatenate(rootfiles, expressions, NEVENT_MAX)
    print('%d events loaded.' % len(uncategorized_events))
    uncategorized_events = uncategorized_events[uncategorized_events['lead_jet_sdmass'] >= 100]
    uncategorized_events = uncategorized_events[uncategorized_events['lead_jet_sdmass'] <= 150]
    print('%d events in mass window [100, 150].' % len(uncategorized_events))
    uncategorized_events['H2BVSQCD'] = 1.0 / (1.0 + uncategorized_events['score_label_QCD'] / uncategorized_events['score_label_H2B'])

    for category in categories:
        events[category] = uncategorized_events[uncategorized_events['label_' + category] == True]
        events[category]['weight'] = weights[category] / len(events[category])

events = { }
prediction['none'] = events
for category in categories:
    events[category] = ak.copy(prediction['lite'][category])
    events[category]['H2BVSQCD'] = 1.0 / (1.0 + events[category]['lead_jet_probQCD'] / events[category]['lead_jet_probHbb'])

def savefig(path, *args, **kwargs):
    print('Saving to %s...' % path)
    plt.savefig(os.path.join('plot' + POSTFIX, path), *args, **kwargs)

def get_signif(s, b):
    return np.sqrt(np.maximum(2 * ((s + b) * np.log(np.maximum(1 + s / (b + (s == 0)), 1)) - s), 0))

def compute_background_suppression(events, min_H2BVSQCD):
    b_all, b_cut = 0.0, 0.0
    for category, category_events in events.items():
        if category == SIGNAL: continue
        b_all += ak.sum(category_events['weight'])
        b_cut += ak.sum(category_events[category_events['H2BVSQCD'] >= min_H2BVSQCD]['weight'])
    return b_cut / b_all

def compute_min_H2BVSQCD(events, bs_exp, l=0.0, r=1.0):
    while l != r:
        m = (l + r) / 2
        if m == r: m = l
        bs = compute_background_suppression(events, m)
        if bs <= bs_exp:  # accepted
            r = m
        else:  # refused
            if m == l: return r
            l = m
    return l

def compute_significance(events, min_H2BVSQCD):
    s_cut, b_cut = 0.0, 0.0
    for category, category_events in events.items():
        if category == SIGNAL:
            s_cut += ak.sum(category_events[category_events['H2BVSQCD'] >= min_H2BVSQCD]['weight'])
        else:
            b_cut += ak.sum(category_events[category_events['H2BVSQCD'] >= min_H2BVSQCD]['weight'])
    return get_signif(s_cut, b_cut)

def roc(events, *args, **kwargs):
    n = 51
    bss = np.empty(n)
    signifs = np.empty(n)
    for i, bs_exp in enumerate(np.logspace(-4, 0, n)):
        min_H2BVSQCD = compute_min_H2BVSQCD(events, bs_exp)
        bs = compute_background_suppression(events, min_H2BVSQCD)
        signif = compute_significance(events, min_H2BVSQCD)
        print('%.6f\t%.6f' % (bs, signif))
        bss[i] = bs
        signifs[i] = signif
    return plt.plot(bss, signifs, *args, **kwargs)

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
