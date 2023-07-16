#!/bin/bash

set -e

(date
../../bin/part-cmp \
    stop2b1000_neutralino300_prod1_weaver.root \
    stop2b1000_neutralino300_prod1_partgpu.gz
date) 2>&1 | tee stop2b1000_neutralino300_prod1_cmp.log

echo "Done!"
