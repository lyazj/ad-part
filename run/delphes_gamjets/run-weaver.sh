#!/bin/bash

set -e

(date
../../bin/part-weaver \
    delphes_gamjets_ntuple.root \
    delphes_gamjets_weaver.root
date) 2>&1 | tee delphes_gamjets_weaver.log

echo "Done!"
