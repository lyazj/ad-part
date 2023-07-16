#!/bin/bash

set -e

(date
../../bin/flatten \
    /home/pku/licq/pheno/anomdet/gen/delphes_output/delphes_delphes_wjets.root \
    delphes_delphes_wjets_flatten.gz
date) 2>&1 | tee delphes_delphes_wjets_flatten.log

echo "Done!"
