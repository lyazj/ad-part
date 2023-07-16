#!/bin/bash

#NPROC=$(cat /proc/cpuinfo | grep MHz | wc -l)
NPROC=100

set -e

(date
rm -rf delphes_njets_flatten_*.gz
for i in $(seq 1 100); do
    if [ ${NPROC} = 0 ]; then
        wait -n || :
    else
        let NPROC-=1 || :
    fi
    echo ${i}
    ../../bin/flatten 0 \
        delphes_njets_flatten_$(printf %06d ${i}).gz \
        delphes_njets_flatten_$(printf %06d ${i})_leptons.gz \
        delphes_njets_flatten_$(printf %06d ${i})_events.gz \
        /home/pku/licq/pheno/anomdet/gen/delphes_output/230919_JetClass_mod_card/njets/delphes_${i}.root \
        &> delphes_njets_flatten_$(printf %06d ${i}).log &
done
wait
gzip -cd delphes_njets_flatten_??????.gz | gzip > delphes_njets_flatten.gz
gzip -cd delphes_njets_flatten_??????_leptons.gz | gzip > delphes_njets_flatten_leptons.gz
gzip -cd delphes_njets_flatten_??????_events.gz | gzip > delphes_njets_flatten_events.gz
date) 2>&1 | tee delphes_njets_flatten.log

echo "Done!"
