#!/bin/bash

set -e

(date
../../bin/part-weaver \
    delphes_njets_ntuple.root \
    delphes_njets_weaver.root
date) 2>&1 | tee delphes_njets_weaver.log

echo "Done!"
