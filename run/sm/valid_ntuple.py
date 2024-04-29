#!/usr/bin/env python3
import uproot

tree = uproot.open('test/TTbar/events_delphes_0_ntuple.root:tree')
events = tree.arrays()[[0,2,5,9,11,15,17,19,21,23]]

ref_tree = uproot.open('/publicfs/cms/user/licq/condor_output/sm/TTbar_ntuple/ntuples_0.root:tree')
ref_events = ref_tree.arrays()[:10]

for event, ref_event in zip(events, ref_events):
    print('-' * 80, end='\n\n')
    for i in range(5):
        print(*['%s=%s' % (field, event[field][i]) for field in sorted(event.fields) if field[:5] == 'part_'], sep='\t', end='\n')
        print(*['%s=%s' % (field, ref_event[field][i]) for field in sorted(ref_event.fields) if field[:5] == 'part_'], sep='\t', end='\n\n')
    print('-' * 80, end='\n\n')
