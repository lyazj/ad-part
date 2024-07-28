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

POSTFIX = re.search(r'(?:^|/)plot([^/]*)\.py$', __file__).group(1) or '_default'
FOLDER = 'plot' + POSTFIX
os.makedirs(FOLDER, exist_ok=True)

# Transcript printed content to a same-name log file.
logfile = open(FOLDER + '.log', 'w')
def print(*args, **kwargs): builtins.print(*args, **kwargs); builtins.print(*args, **{**kwargs, 'file': logfile})

plt.style.use(hep.style.CMS)
gs = gridspec.GridSpec(2, 1, height_ratios=[4, 1])

SIGNAL = 'gghh'
PRED = '../../run/2H4B-raw/predict_output/pred%s.root' % POSTFIX
PRED_GGHH = '../../run/gghh-raw/pred%s_%%s.root' % POSTFIX
NEVENT_MAX = None
#NEVENT_MAX = 10000

event_expressions = None
labels = {
    'QCD':   r'QCD',
    'VJets': r'V+jets',
    'TTbar': r'$t\bar{t}$',
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

events = { }

rootfiles = [PRED + ':Events']
print('ROOT files:', *rootfiles, sep='\n  - ')
print('Loading events...')
uncategorized_events = concatenate(rootfiles, expressions, NEVENT_MAX)
print('%d events loaded.' % len(uncategorized_events))
uncategorized_events['2H4BVSQCD'] = 1.0 / (1.0 + uncategorized_events['score_label_QCD'] / uncategorized_events['score_label_2H4B'])

for category in weights:
    events[category] = uncategorized_events[uncategorized_events['label_' + category] == True]
    events[category]['weight'] = weights[category] / len(events[category])

for category in weights_gghh:
    rootfiles = [PRED_GGHH % category + ':Events']
    print('ROOT files:', *rootfiles, sep='\n  - ')
    print('Loading events...')
    events[category] = concatenate(rootfiles, expressions, NEVENT_MAX)
    print('%d events loaded.' % len(events[category]))
    events[category]['2H4BVSQCD'] = 1.0 / (1.0 + events[category]['score_label_QCD'] / events[category]['score_label_2H4B'])

raw_events = events
del events

def reweight(kl, kt):
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

def figure(*args, **kwargs):
    fig = plt.figure(*args, **kwargs)
    fig.add_subplot(gs[0])
    return fig

def scale(hists, labs):
    for i, (count, _) in enumerate(hists):
        value = count.mean()
        if value >= 1.0: continue
        sf = 10**int(np.ceil(-np.log10(value)))
        count *= sf
        labs[i] += r' ($\times' + str(sf) + r'$)'

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
    labs = [labels[cate] for cate in cates]
    sig_labs = [labels[cate] for cate in sig_cates]
    scale(hists, labs)
    scale(sig_hists, sig_labs)
    hep.histplot(hists,     stack=True , histtype='fill', label=labs,     edgecolor='black', linewidth=0.5)
    hep.histplot(sig_hists, stack=False, histtype='step', label=sig_labs, color='black')

def savefig(path, *args, **kwargs):
    print('Saving to %s...' % path)
    plt.savefig(os.path.join('plot' + POSTFIX, path), *args, **kwargs)

def get_signif(s, b):
    return s / np.sqrt(b + 1)

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
    plt.plot([], [], 'k--', label='optimal (%.5f)' % signif_max)
    plt.legend()

events = reweight(kl=1.0, kt=1.0)

plt.figure(figsize=(12, 9), dpi=150)
pt_bins = np.linspace(0, 1200, 51)
pt_hists = [np.histogram(events[category]['lead_jet_pt'], pt_bins, density=True) for category in categories]
hep.histplot(pt_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'$p_\mathrm{T}$ [GeV]'); plt.ylabel('Density')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('pt-density.pdf')
plt.close()

plt.figure(figsize=(12, 9), dpi=150)
sdmass_bins = np.linspace(50, 250, 21)
sdmass_hists = [np.histogram(events[category]['lead_jet_sdmass'], sdmass_bins, density=True) for category in categories]
hep.histplot(sdmass_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'Leading fat jet soft-drop mass $m_\mathrm{SD}$ [GeV]'); plt.ylabel('Density')
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
TH4BVSQCD_bins = np.linspace(0, 1, 51)
TH4BVSQCD_hists = [np.histogram(cut_events[category]['2H4BVSQCD'], TH4BVSQCD_bins, density=True) for category in categories]
hep.histplot(TH4BVSQCD_hists, histtype='step', label=[labels[cate] for cate in categories])
plt.xlabel(r'2H4BVSQCD'); plt.ylabel('Density')
plt.legend(); plt.grid(); plt.tight_layout(); savefig('2H4BVSQCD-100-150-density.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
TH4BVSQCD_bins = np.linspace(0, 1, 51)
TH4BVSQCD_hists = [np.histogram(cut_events[category]['2H4BVSQCD'], TH4BVSQCD_bins, weights=cut_events[category]['weight']) for category in categories]
histplot(TH4BVSQCD_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(TH4BVSQCD_hists, categories)
plt.xlabel(r'2H4BVSQCD'); plt.ylabel('Density'); plt.grid()
plt.tight_layout(); savefig('2H4BVSQCD-100-150.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
TH4BVSQCD_bins = np.linspace(0.9, 1, 51)
TH4BVSQCD_hists = [np.histogram(cut_events[category]['2H4BVSQCD'], TH4BVSQCD_bins, weights=cut_events[category]['weight']) for category in categories]
histplot(TH4BVSQCD_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(TH4BVSQCD_hists, categories)
plt.xlabel(r'2H4BVSQCD'); plt.ylabel('Density'); plt.grid()
plt.tight_layout(); savefig('2H4BVSQCD-100-150-0.9-1.0.pdf')
plt.close()

fig = figure(figsize=(12, 11.25), dpi=150)
sdmass_bins = np.linspace(50, 250, 21)
sdmass_hists = [np.histogram(events[category]['lead_jet_sdmass'], sdmass_bins, weights=events[category]['weight']) for category in categories]
histplot(sdmass_hists, categories)
plt.ylabel('Events'); plt.yscale('log'); plt.legend(); plt.grid()
plt.gca().set_xticklabels([]); fig.add_subplot(gs[1])
signif(sdmass_hists, categories)
plt.xlabel(r'Leading fat jet soft-drop mass $m_\mathrm{SD}$ [GeV]'); plt.ylabel('Significance'); plt.grid()
plt.tight_layout(); savefig('sdmass.pdf')
plt.close()

# Apply 2H4BVSQCD cut.
for threshold in [0, 0.9985]:
    cut_events = { }
    for category in events:
        e = events[category]
        e = e[e['2H4BVSQCD'] >= threshold]
        cut_events[category] = e
    fig = plt.figure(figsize=(12, 9), dpi=150)
    sdmass_bins = np.linspace(50, 250, 21)
    sdmass_hists = [np.histogram(cut_events[category]['lead_jet_sdmass'], sdmass_bins, weights=cut_events[category]['weight']) for category in categories]
    histplot(sdmass_hists, categories)
    s, b = 0.0, 0.0
    for category in cut_events:
        e = cut_events[category]
        e = e[e['lead_jet_sdmass'] >= 100]
        e = e[e['lead_jet_sdmass'] <= 150]
        if category == SIGNAL: s += np.sum(e['weight'])
        else: b += np.sum(e['weight'])
    signif = get_signif(s, b)
    print('thr=%.4f s=%.3f b=%.3f signif=%.5f' % (threshold, s, b, signif))
    plt.plot([], [], 'k-', label='significance: %.5f' % signif)
    plt.xlabel(r'Leading fat jet soft-drop mass $m_\mathrm{SD}$ [GeV]'); plt.ylabel('Events'); plt.yscale('log'); plt.legend(loc='upper right'); plt.grid()
    plt.tight_layout(); savefig('sdmass-%.4f.pdf' % threshold)
    plt.close()
