#!/bin/bash

set -e

(date
../../bin/ntuple \
    delphes_gamjets_flatten.gz \
    delphes_gamjets_ntuple.root
date) 2>&1 | tee delphes_gamjets_ntuple.log

echo "Done!"
