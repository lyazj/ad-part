#!/bin/bash

set -e

NFILE=1
PF_FILES=
for DIR in ../sm/data4/*; do
    for((IFILE=0; IFILE<NFILE; ++IFILE)); do
        if [ -z "${PF_FILES}" ]; then
            PF_FILES=${DIR}/${IFILE}.gz
        else
            PF_FILES=${PF_FILES}:${DIR}/${IFILE}.gz
        fi
    done
done
CF_FILES="$(echo ${PF_FILES} | sed 's@\.gz@_part_hid.gz@g')"

(date
rm -rf tiny
stdbuf -oL python3 -u ../../bin/tiny-train ${PF_FILES} ${CF_FILES} tiny - 8192
date) 2>&1 | tee train.log

echo "Done!"
