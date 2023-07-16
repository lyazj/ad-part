#!/bin/bash

set -e

(date
../../bin/flatten \
    /home/pku/licq/pheno/anomdet/gen/delphes_output/delphes_stop2b1000_neutralino300_prod1.root \
    delphes_stop2b1000_neutralino300_prod1_flatten.gz
date) 2>&1 | tee delphes_stop2b1000_neutralino300_prod1_flatten.log

echo "Done!"
