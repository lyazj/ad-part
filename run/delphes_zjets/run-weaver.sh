#!/bin/bash

set -e

(date
../../bin/part-weaver \
    delphes_zjets_ntuple.root \
    delphes_zjets_weaver.root
date) 2>&1 | tee delphes_zjets_weaver.log

echo "Done!"
