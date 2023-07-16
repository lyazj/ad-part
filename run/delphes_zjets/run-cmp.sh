#!/bin/bash

set -e

(date
../../bin/part-cmp \
    delphes_zjets_weaver.root \
    delphes_zjets_partgpu.gz
date) 2>&1 | tee delphes_zjets_cmp.log

echo "Done!"
