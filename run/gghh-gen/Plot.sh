#!/bin/bash

source ../../../env.sh
NPROC=$(nproc)
IPROC=0

for FILE in $(ls -v samples/*/*.root); do
    if [ ${IPROC} = ${NPROC} ]; then
        wait -n
    else
        let IPROC+=1
    fi
    echo root -b -l -q 'Plot.C("'"${FILE}"'", "'"${FILE/samples/plot}"'")'
    root -b -l -q 'Plot.C("'"${FILE}"'", "'"${FILE/samples/plot}"'")' &
done
wait
for DIR in $(ls -dv plot/*); do
    hadd -f "${DIR}".root "${DIR}"/*.root
done
