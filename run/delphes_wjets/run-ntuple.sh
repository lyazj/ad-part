#!/bin/bash

set -e

(date
../../bin/ntuple \
    delphes_wjets_flatten.gz \
    delphes_wjets_ntuple.root
date) 2>&1 | tee delphes_wjets_ntuple.log

echo "Done!"
