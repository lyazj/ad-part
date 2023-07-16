#!/bin/bash

#NPROC=$(cat /proc/cpuinfo | grep MHz | wc -l)
NPROC=100

set -e

(date
rm -rf stop2b1000_neutralino300_prod1_flatten_*.gz
for i in $(seq 1 100); do
    if [ ${NPROC} = 0 ]; then
        wait -n || :
    else
        let NPROC-=1 || :
    fi
    echo ${i}
    ../../bin/flatten 1 \
        stop2b1000_neutralino300_prod1_flatten_$(printf %06d ${i}).gz \
        stop2b1000_neutralino300_prod1_flatten_$(printf %06d ${i})_leptons.gz \
        stop2b1000_neutralino300_prod1_flatten_$(printf %06d ${i})_events.gz \
        input/stop2b1000_neutralino300_prod1_$(printf %06d ${i}).root \
        &> stop2b1000_neutralino300_prod1_flatten_$(printf %06d ${i}).log &
done
wait
gzip -cd stop2b1000_neutralino300_prod1_flatten_??????.gz | gzip > stop2b1000_neutralino300_prod1_flatten.gz
gzip -cd stop2b1000_neutralino300_prod1_flatten_??????_leptons.gz | gzip > stop2b1000_neutralino300_prod1_flatten_leptons.gz
gzip -cd stop2b1000_neutralino300_prod1_flatten_??????_events.gz | gzip > stop2b1000_neutralino300_prod1_flatten_events.gz
date) 2>&1 | tee stop2b1000_neutralino300_prod1_flatten.log

echo "Done!"
