#!/bin/bash

set -e

(date
../../bin/part-cmp \
    delphes_gamjets_weaver.root \
    delphes_gamjets_partgpu.gz
date) 2>&1 | tee delphes_gamjets_cmp.log

echo "Done!"
