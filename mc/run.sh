#!/bin/bash

mkdir -p run
i=0
while true; do
    let i+=1
    DIR=run/$(printf %02d ${i})
    if [ -d ${DIR} ]; then continue; fi
    echo "Preparing directory: ${DIR}"
    cp -r proc ${DIR}
    sed -i 's@\S*\(\s*=\s*iseed\)@'${i}'\1@g' ${DIR}/Cards/run_card.dat
    sed "s@launch proc@launch ${DIR}@" run.dat > ${DIR}.dat
    stdbuf -oL mg5_aMC ${DIR}.dat 2>&1 | tee ${DIR}.log
    break
done
