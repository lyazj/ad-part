#!/bin/bash

set -e

(date
../../bin/ntuple \
    delphes_zjets_flatten.gz \
    delphes_zjets_ntuple.root
date) 2>&1 | tee delphes_zjets_ntuple.log

echo "Done!"
