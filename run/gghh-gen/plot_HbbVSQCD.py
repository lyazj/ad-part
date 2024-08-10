#!/usr/bin/env python3

import sys
sys.path.insert(0, '../../src/python3/')
import adjet

import os
import re
import numpy as np

evt_files = os.popen('ls -v rawdata/ggHH_kl_1_kt_1/events_delphes_*_events.gz').read().strip().split('\n')
pf_files = [ re.sub(r'_events\.gz$', '.gz', evt_file) for evt_file in evt_files ]
cf_files = [ re.sub(r'_events\.gz$', '_part_out.gz', evt_file) for evt_file in evt_files ]

HbbVSQCDs = [[], [], []]  # [nHbb]
GEN_QCD = 0
GEN_HBB = 1
PART_HBB = np.arange(0, 1)
PART_QCD = np.arange(161, 188)

for pf_file, cf_file, evt_file in zip(pf_files, cf_files, evt_files):
    print('Processing', pf_file)
    pf_data = adjet.ADPFData(np.fromgz(pf_file, dtype=adjet.Feature), reduced=False)
    cf_data = adjet.ADCFData(np.fromgz(cf_file, dtype=adjet.Feature))
    evt_data = adjet.ADEVTData(np.fromgz(evt_file, dtype=adjet.Feature))
    ijet = 0
    for evt in evt_data.data:
        evt = adjet.ADEvent(evt)
        njet = int(evt.njet)
        if njet >= 2:
            labels = pf_data.data[ijet : ijet + 2, adjet.JET_LABEL]
            Hbb_scores = cf_data.data[ijet : ijet + 2, PART_HBB].sum(axis=-1)
            QCD_scores = cf_data.data[ijet : ijet + 2, PART_QCD].sum(axis=-1)
            nHbb = (labels == GEN_HBB).sum()
            HbbVSQCD_scores = Hbb_scores / (Hbb_scores + QCD_scores)
            HbbVSQCDs[nHbb].append(HbbVSQCD_scores)
        ijet += njet
    assert ijet == len(pf_data.data)

import matplotlib.pyplot as plt

plt.figure()
plt.plot([], [], linestyle='', label=r'gg$\to$HH(HH$\to$4b)')
for nHbb, HbbVSQCD in enumerate(HbbVSQCDs):
    if not HbbVSQCD: continue
    plt.hist(np.array(HbbVSQCD)[:,0], bins=np.linspace(0, 1, 51), histtype='step', label=f'{nHbb} gen-Hbb')
plt.xlabel('Leading jet Hbb/(Hbb+QCD)')
plt.ylabel('Events')
plt.legend()
plt.tight_layout()
plt.savefig('plot_HbbVSQCD.pdf')
plt.close()

plt.figure()
plt.plot([], [], linestyle='', label=r'gg$\to$HH(HH$\to$4b)')
for nHbb, HbbVSQCD in enumerate(HbbVSQCDs):
    if not HbbVSQCD: continue
    plt.hist(np.array(HbbVSQCD)[:,1], bins=np.linspace(0, 1, 51), histtype='step', label=f'{nHbb} gen-Hbb')
plt.xlabel('Subleading jet Hbb/(Hbb+QCD)')
plt.ylabel('Events')
plt.legend()
plt.tight_layout()
plt.savefig('plot_HbbVSQCD_sub.pdf')
plt.close()

plt.figure()
plt.plot([], [], linestyle='', label=r'gg$\to$HH(HH$\to$4b)')
for nHbb, HbbVSQCD in enumerate(HbbVSQCDs):
    if not HbbVSQCD: continue
    plt.hist(np.min(HbbVSQCD, axis=-1), bins=np.linspace(0, 1, 51), histtype='step', label=f'{nHbb} gen-Hbb')
plt.xlabel('HH4B/(HH4B+QCD)')
plt.ylabel('Events')
plt.legend()
plt.tight_layout()
plt.savefig('plot_2H4BVSQCD.pdf')
plt.close()
