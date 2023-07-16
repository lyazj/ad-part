#!/bin/bash

#NPROC=$(cat /proc/cpuinfo | grep MHz | wc -l)
NPROC=100

set -e

(date
rm -rf delphes_zjets_flatten_*.gz
for i in $(seq 1 10000); do
    [ -e input/delphes_zjets_$(printf %06d ${i}).root ] || break
    if [ ${NPROC} = 0 ]; then
        wait -n || :
    else
        let NPROC-=1 || :
    fi
    echo ${i}
    ../../bin/flatten 0 \
        delphes_zjets_flatten_$(printf %06d ${i}).gz \
        delphes_zjets_flatten_$(printf %06d ${i})_leptons.gz \
        delphes_zjets_flatten_$(printf %06d ${i})_events.gz \
        input/delphes_zjets_$(printf %06d ${i}).root \
        &> delphes_zjets_flatten_$(printf %06d ${i}).log &
done
wait
gzip -cd delphes_zjets_flatten_??????.gz | gzip > delphes_zjets_flatten.gz
gzip -cd delphes_zjets_flatten_??????_leptons.gz | gzip > delphes_zjets_flatten_leptons.gz
gzip -cd delphes_zjets_flatten_??????_events.gz | gzip > delphes_zjets_flatten_events.gz
date) 2>&1 | tee delphes_zjets_flatten.log

echo "Done!"
