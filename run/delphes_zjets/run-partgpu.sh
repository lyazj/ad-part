#!/bin/bash

set -e

(date
../../bin/part-gpu \
    delphes_zjets_flatten.gz \
    delphes_zjets_partgpu.gz
date) 2>&1 | tee delphes_zjets_partgpu.log

echo "Done!"
