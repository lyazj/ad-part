#!/bin/bash

mkdir -p data
CMD="../../bin/sample rawdata data 1000 &> run_sample.log"
echo "${CMD}"
eval "${CMD}"
