#!/bin/bash

set -e

PF_FILES="$(ls ../*/*_flatten.gz | xargs echo | sed 's@ @:@g')"
CF_FILES="-"

(date
stdbuf -oL python3 -u ../../bin/tiny-train ${PF_FILES} ${CF_FILES} tiny
date) 2>&1 | tee train.log

echo "Done!"
