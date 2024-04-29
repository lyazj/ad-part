#!/usr/bin/env python3
import sys
sys.path.insert(0, '../../src/python3')
import adjet
import numpy as np
import uproot
import matplotlib.pyplot as plt

evts = np.fromgz('test/TTbar/events_delphes_0_events.gz', dtype='float32').reshape(-1, adjet.NFEAT_EVT)
jets = np.fromgz('test/TTbar/events_delphes_0.gz', dtype='float32').reshape(-1, adjet.NFEAT_JET + adjet.NPAR_JET * adjet.NFEAT_PAR)
outs = np.fromgz('test/TTbar/events_delphes_0_part_out.gz', dtype='float32').reshape(-1, adjet.NRSLT_CLS)
print('%d events, %d jets, %d jets predicted' % (len(evts), len(jets), len(outs)))

ref_tree = uproot.open('/publicfs/cms/user/licq/condor_output/sm/TTbar_ntuple/ntuples_0.root:tree')
ref_evts = ref_tree.arrays()
ref_evts = ref_evts[:len(evts)]

matched_jets = []
matched_outs = []
ijet = 0
iref_jet = 0
for evt, ref_evt in zip(evts, ref_evts):
    njet = int(evt[adjet.EVT_NJET])

    jet_pt = jets[ijet : ijet + njet, adjet.JET_PT]
    ref_jet_pt = ref_evts['jet_pt'][iref_jet]
    jet_pt_diff = np.abs(jet_pt - ref_jet_pt)
    ibest = ijet + jet_pt_diff.argmin()
    if jet_pt_diff[ibest - ijet] > 0.001: print('jet_pt_diff[ibest - ijet] > 0.001:', iref_jet)

    jet = jets[ibest]
    matched_jets.append(jet)
    out = outs[ibest]
    matched_outs.append(out)

    ijet += njet
    iref_jet += 1

outs = np.array(matched_outs)
ref_outs = ref_evts['jet_probs'].to_numpy()
diffs = outs - ref_outs

selected_outs = []
selected_ref_outs = []

for i, m in enumerate(diffs.max(1)):
    if m <= 0.05:
        selected_outs.append(outs[i])
        selected_ref_outs.append(ref_outs[i])
        continue
    print(i, m, '\n' + '-' * 80)

    jet = adjet.ADJet(matched_jets[i], reduced=False)
    par = jet.par[:int(jet.npar)]
    print('npar=%d  npar_ref=%d' % (len(par), len(ref_evts['part_pt'][i])))
    for j in range(min(len(par), len(ref_evts['part_pt'][i]))):
        print(*['%s=%s' % (adjet.PAR_FEAT_NAME[k], par[j,k]) for k in range(adjet.NFEAT_PAR)], sep='\t', end='\n')
        print(*['%s=%s' % (field, ref_evts[field][i][j]) for field in sorted(ref_evts.fields) if field[:5] == 'part_'], sep='\t', end='\n\n')
    print()

selected_outs = np.array(selected_outs)
selected_ref_outs = np.array(selected_ref_outs)
plt.plot(selected_ref_outs.flatten(), selected_outs.flatten(), '.')
plt.xlabel('reference value')
plt.ylabel('output value')
plt.tight_layout()
plt.savefig('valid_part.pdf')
