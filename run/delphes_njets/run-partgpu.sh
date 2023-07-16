#!/bin/bash

set -e

(date
../../bin/part-gpu \
    delphes_njets_flatten.gz \
    delphes_njets_partgpu.gz
date) 2>&1 | tee delphes_njets_partgpu.log

echo "Done!"
