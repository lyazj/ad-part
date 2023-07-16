#!/bin/bash

set -e

(date
../../bin/part-weaver \
    stop2b1000_neutralino300_prod1_ntuple.root \
    stop2b1000_neutralino300_prod1_weaver.root
date) 2>&1 | tee stop2b1000_neutralino300_prod1_weaver.log

echo "Done!"
