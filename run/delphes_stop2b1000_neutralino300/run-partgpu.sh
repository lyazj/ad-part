#!/bin/bash

set -e

(date
../../bin/part-gpu \
    stop2b1000_neutralino300_prod1_flatten.gz \
    stop2b1000_neutralino300_prod1_partgpu.gz
date) 2>&1 | tee stop2b1000_neutralino300_prod1_partgpu.log

echo "Done!"
