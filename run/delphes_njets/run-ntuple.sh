#!/bin/bash

set -e

(date
../../bin/ntuple \
    delphes_njets_flatten.gz \
    delphes_njets_ntuple.root
date) 2>&1 | tee delphes_njets_ntuple.log

echo "Done!"
