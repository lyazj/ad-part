#!/bin/bash

set -e

(date
../../bin/part-weaver \
    delphes_wjets_ntuple.root \
    delphes_wjets_weaver.root
date) 2>&1 | tee delphes_wjets_weaver.log

echo "Done!"
