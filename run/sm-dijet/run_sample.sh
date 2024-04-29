#!/bin/bash

export SAMPLE_START_LABEL=5

mkdir -p data
CMD="../../bin/sample rawdata data 100 &> run_sample.log"
echo "${CMD}"
eval "${CMD}"
