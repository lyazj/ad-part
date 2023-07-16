#!/bin/bash

set -e

(date
../../bin/part-cmp \
    delphes_njets_weaver.root \
    delphes_njets_partgpu.gz
date) 2>&1 | tee delphes_njets_cmp.log

echo "Done!"
