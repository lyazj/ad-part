#!/bin/bash

I=0
for FILE in $(cat remote_run.txt); do
    FILE=${FILE/samples/rawdata}
    FILE=${FILE/.root/.gz}
    if [ ! -f ${FILE} ]; then
        hep_sub remote_run.sh -argu ${I} -n 1 -o log/${I}_1.log -e log/${I}_2.log
    fi
    let I+=1
done
