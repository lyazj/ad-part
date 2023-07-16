#!/bin/bash

set -e

PF_FILES="$(ls ../*/*_flatten.gz | xargs echo | sed 's@ @:@g')"
CF_FILES="-"

(date
rm -rf tiny
stdbuf -oL python3 -u ../../bin/tiny-train ${PF_FILES} ${CF_FILES} tiny - 8192
date) 2>&1 | tee train.log

echo "Done!"
