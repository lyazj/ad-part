#!/bin/bash

set -e

(date
../../bin/ntuple \
    stop2b1000_neutralino300_prod1_flatten.gz \
    stop2b1000_neutralino300_prod1_ntuple.root
date) 2>&1 | tee stop2b1000_neutralino300_prod1_ntuple.log

echo "Done!"
