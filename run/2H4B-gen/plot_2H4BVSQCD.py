#!/usr/bin/env python3

import sys
sys.path.insert(0, '../../src/python3/')
import adjet

import os
import re
import numpy as np

pf_files = os.popen('ls -v rawdata/DiHiggsTo4B/events_delphes_*.gz').read().strip().split('\n')
cf_files = [ re.sub(r'\.gz$', '_part.gz', pf_file) for pf_file in pf_files ]
evt_files = [ re.sub(r'\.gz$', '_events.gz', pf_file) for pf_file in pf_files ]

HH4BVSQCDs = [[], [], []]  # [nHBB]
GEN_QCD = 0
GEN_HBB = 1
PART_HBB = np.arange(0, 1)
PART_QCD = np.arange(161, 188)

for pf_file, cf_file, evt_file in zip(pf_files, cf_files, evt_files):
    pf_data = adjet.ADPFData(np.fromgz(pf_file, dtype=adjet.Feature), reduced=False)
    cf_data = adjet.ADCFData(np.fromgz(cf_file, dtype=adjet.Feature))
    evt_data = adjet.ADEVTData(np.fromgz(evt_file, dtype=adjet.Feature))
    ijet = 0
    for evt in evt_data:
        evt = adjet.ADEvent(evt)
        njet = evt.njet
        if njet >= 2:
            labels = pf_data.data[ijet : ijet + 2, adjet.JET_LABEL]
            HBB_scores = cf_data.data[ijet : ijet + 2, PART_HBB].sum(axis=-1)
            QCD_scores = cf_data.data[ijet : ijet + 2, PART_QCD].sum(axis=-1)
            nHBB = (labels == GEN_HBB).sum()
            HBBVSQCD_scores = HBB_scores / (HBB_scores + QCD_scores)
            HBBVSQCD_score = HBBVSQCD_scores.min()
            HBBVSQCDs[nHBB].append(HBBVSQCD_score)
        ijet += njet
    assert ijet == len(pf_data.data)

import matplotlib.pyplot as plt
plt.plot([], [], style='', label=r'HH \to 4b')
plt.hist(HH4BVSQCDs, bins=np.linspace(0, 1, 51), density=True, histtype='step', label=['0 gen-HBB', '1 gen-HBB', '2 gen-HBB'])
plt.xlabel('HBB / (HBB + QCD)')
plt.ylabel('Density')
plt.legend()
plt.tight_layout()
plt.savefig('plot_2H4BVSQCD.pdf')
