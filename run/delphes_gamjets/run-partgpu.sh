#!/bin/bash

set -e

(date
../../bin/part-gpu \
    delphes_gamjets_flatten.gz \
    delphes_gamjets_partgpu.gz
date) 2>&1 | tee delphes_gamjets_partgpu.log

echo "Done!"
