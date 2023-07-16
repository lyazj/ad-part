#!/bin/bash

set -e

(date
../../bin/part-gpu \
    delphes_wjets_flatten.gz \
    delphes_wjets_partgpu.gz
date) 2>&1 | tee delphes_wjets_partgpu.log

echo "Done!"
