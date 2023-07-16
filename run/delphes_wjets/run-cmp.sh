#!/bin/bash

set -e

(date
../../bin/part-cmp \
    delphes_wjets_weaver.root \
    delphes_wjets_partgpu.gz
date) 2>&1 | tee delphes_wjets_cmp.log

echo "Done!"
