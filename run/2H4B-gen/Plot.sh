#!/bin/bash

source ../../../env.sh
NPROC=$(nproc)
IPROC=0

for FILE in $(ls -v samples/DiHiggsTo4B); do
    if [ ${IPROC} = ${NPROC} ]; then
        wait -n
    else
        let IPROC+=1
    fi
    echo root -b -l -q 'Plot.C("samples/DiHiggsTo4B/'"${FILE}"'", "plot/DiHiggsTo4B/'"${FILE}"'")'
    root -b -l -q 'Plot.C("samples/DiHiggsTo4B/'"${FILE}"'", "plot/DiHiggsTo4B/'"${FILE}"'")' &
done
wait
hadd -f plot/DiHiggsTo4B.root $(ls -v plot/DiHiggsTo4B/*.root)
