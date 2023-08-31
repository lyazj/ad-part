#!/bin/bash

set -e

PF_FILES="$(ls ../*/*_flatten.gz | xargs echo | sed 's@ @:@g')"
CF_FILES="$(ls ../*/*_partgpu.gz | xargs echo | sed 's@ @:@g')"

(date
stdbuf -oL ../../bin/tiny-train ${PF_FILES} ${CF_FILES} tiny
date) 2>&1 | tee train.log

echo "Done!"
