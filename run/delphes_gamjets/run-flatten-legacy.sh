#!/bin/bash

set -e

(date
../../bin/flatten \
    /home/pku/licq/pheno/anomdet/gen/delphes_output/delphes_delphes_gamjets.root \
    delphes_delphes_gamjets_flatten.gz
date) 2>&1 | tee delphes_delphes_gamjets_flatten.log

echo "Done!"
