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
import matplotlib.gridspec as gridspec

# Transcript printed content to a same-name log file.
logfile = open(re.sub(r'\.py$', '.log', __file__), 'w')
def print(*args, **kwargs): builtins.print(*args, **kwargs); builtins.print(*args, **{**kwargs, 'file': logfile})

plt.style.use(hep.style.CMS)
gs = gridspec.GridSpec(2, 1, height_ratios=[4, 1])

SIGNAL = 'H2B'
POSTFIX = re.search(r'(?:^|/)plot([^/]*)\.py$', __file__).group(1) or '_default'
os.makedirs('plot' + POSTFIX, exist_ok=True)
PRED = '../../run/H2B-raw/predict_output/pred%s.root' % POSTFIX
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
    'H2B':   48.5 * 0.582,
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

events = { }

rootfiles = [PRED + ':Events']
print('ROOT files:', *rootfiles, sep='\n  - ')
print('Loading events...')
uncategorized_events = concatenate(rootfiles, expressions, NEVENT_MAX)
print('%d events loaded.' % len(uncategorized_events))
uncategorized_events['H2BVSQCD'] = 1.0 / (1.0 + uncategorized_events['score_label_QCD'] / uncategorized_events['score_label_H2B'])

for i, category in enumerate(categories):
    events[category] = uncategorized_events[uncategorized_events['label_' + category] == True]
    events[category]['weight'] = weights[category] / len(events[category])

def figure(*args, **kwargs):
    fig = plt.figure(*args, **kwargs)
    fig.add_subplot(gs[0])
    return fig

def histplot(hists, cates):
    counts     = [hist[0] for (hist, cate) in zip(hists, cates) if cate != SIGNAL]
    bins       = [hist[1] for (hist, cate) in zip(hists, cates) if cate != SIGNAL]
    sig_hists  = [hist    for (hist, cate) in zip(hists, cates) if cate == SIGNAL]
    sig_cates  = [SIGNAL]
    cates      = [cate for cate in cates if cate != SIGNAL]
    count_sums = [np.sum(count) for count in counts]
    items = sorted(zip(count_sums, cates, counts, bins))
    cates      = [item[1]            for item in items]
    hists      = [(item[2], item[3]) for item in items]
    hep.histplot(hists,     stack=True , histtype='fill', label=[labels[cate] for cate in cates    ], edgecolor='black', linewidth=0.5)
    hep.histplot(sig_hists, stack=False, histtype='step', label=[labels[cate] for cate in sig_cates], color='black')

def savefig(path, *args, **kwargs):
    print('Saving to %s...' % path)
    plt.savefig(os.path.join('plot' + POSTFIX, path), *args, **kwargs)

def get_signif(s, b):
    return np.sqrt(np.maximum(2 * ((s + b) * np.log(np.maximum(1 + s / (b + (s == 0)), 1)) - s), 0))

def signif(hists, cates):
    sig_hists  = [hist    for (hist, cate) in zip(hists, cates) if cate == SIGNAL]
    hists      = [hist    for (hist, cate) in zip(hists, cates) if cate != SIGNAL]
    bins = sig_hists[0][1]
    for hist in sig_hists + hists:
        if np.any(hist[1] != bins): raise NotImplementedError('rebinning not implemented')
        if hist[0].shape[0] + 1 != hist[1].shape[0]: raise ValueError('incompatible bin count size')
    s_cumsum = np.sum([np.cumsum(np.concatenate([[0], count])) for (count, _) in sig_hists], axis=0)
    b_cumsum = np.sum([np.cumsum(np.concatenate([[0], count])) for (count, _) in hists    ], axis=0)
    signif = np.zeros((len(bins), len(bins)))
    for imin in range(len(bins) - 1):
        for imax in range(imin + 1, len(bins)):
            s = s_cumsum[imax] - s_cumsum[imin]
            b = b_cumsum[imax] - b_cumsum[imin]
            signif[imin, imax] = get_signif(s, b)
    signif_l = signif.max(axis=1)
    signif_u = signif.max(axis=0)
    l = signif_l.argmax()
    u = signif_u.argmax()
    signif_max = signif_l[l]
    plt.plot(bins, signif_l, label='lower')
    plt.plot(bins, signif_u, label='upper')
    plt.plot([bins[l]] * 2, [0, signif_max * 1.2], 'k--')
    plt.plot([bins[u]] * 2, [0, signif_max * 1.2], 'k--')
    plt.plot([], [], 'k--', label='optimal (%.3f)' % signif_max)
    plt.legend()

plt.figure(figsize=(12, 9), dpi=150)
pt_bins = np.linspace(0, 1200, 51)
pt_hists = [np.histogram(events[category]['lead_jet_pt'], pt_bins, density=True) for category in categories]
hep.histplot(pt_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'$p_\mathrm{T}$ [GeV]'); plt.ylabel('Density')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('pt-density.pdf')
plt.close()

plt.figure(figsize=(12, 9), dpi=150)
sdmass_bins = np.linspace(50, 200, 51)
sdmass_hists = [np.histogram(events[category]['lead_jet_sdmass'], sdmass_bins, density=True) for category in categories]
hep.histplot(sdmass_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'Soft Dropped Mass [GeV]'); plt.ylabel('Density')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('sdmass-density.pdf')
plt.close()

# Apply mass window.
cut_events = { }
for category in events:
    e = events[category]
    e = e[e['lead_jet_sdmass'] >= 100]
    e = e[e['lead_jet_sdmass'] <= 150]
    cut_events[category] = e
plt.figure(figsize=(12, 9), dpi=150)
H2BVSQCD_bins = np.linspace(0, 1, 51)
H2BVSQCD_hists = [np.histogram(cut_events[category]['H2BVSQCD'], H2BVSQCD_bins, density=True) for category in categories]
hep.histplot(H2BVSQCD_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'H2BVSQCD'); plt.ylabel('Density')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('H2BVSQCD-100-150-density.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
H2BVSQCD_bins = np.linspace(0, 1, 51)
H2BVSQCD_hists = [np.histogram(cut_events[category]['H2BVSQCD'], H2BVSQCD_bins, weights=cut_events[category]['weight']) for category in categories]
histplot(H2BVSQCD_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(H2BVSQCD_hists, categories)
plt.xlabel(r'H2BVSQCD'); plt.ylabel('Density'); plt.grid()
plt.tight_layout(); savefig('H2BVSQCD-100-150.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
H2BVSQCD_bins = np.linspace(0.9, 1, 51)
H2BVSQCD_hists = [np.histogram(cut_events[category]['H2BVSQCD'], H2BVSQCD_bins, weights=cut_events[category]['weight']) for category in categories]
histplot(H2BVSQCD_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(H2BVSQCD_hists, categories)
plt.xlabel(r'H2BVSQCD'); plt.ylabel('Density'); plt.grid()
plt.tight_layout(); savefig('H2BVSQCD-100-150-0.9-1.0.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
sdmass_bins = np.linspace(50, 200, 51)
sdmass_hists = [np.histogram(events[category]['lead_jet_sdmass'], sdmass_bins, weights=events[category]['weight']) for category in categories]
histplot(sdmass_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(sdmass_hists, categories)
plt.xlabel(r'Soft Dropped Mass [GeV]'); plt.ylabel('Significance'); plt.grid()
plt.tight_layout(); savefig('sdmass.pdf')
plt.close()

# Apply H2BVSQCD cut.
for threshold in [0.5, 0.6, 0.7, 0.8, 0.9]:
    cut_events = { }
    for category in events:
        e = events[category]
        e = e[e['H2BVSQCD'] >= threshold]
        cut_events[category] = e
    fig = figure(figsize=(12, 11.25), dpi=150)
    sdmass_bins = np.linspace(50, 200, 51)
    sdmass_hists = [np.histogram(cut_events[category]['lead_jet_sdmass'], sdmass_bins, weights=cut_events[category]['weight']) for category in categories]
    histplot(sdmass_hists, categories)
    plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
    plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
    signif(sdmass_hists, categories)
    plt.xlabel(r'Soft Dropped Mass [GeV]'); plt.ylabel('Significance'); plt.grid()
    plt.tight_layout(); savefig('sdmass-%.3f.pdf' % threshold)
    plt.close()
