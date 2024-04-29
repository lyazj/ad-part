#!/bin/bash

mkdir -p data
CMD="../../bin/sample rawdata data 100 &> run_sample.log"
echo "${CMD}"
eval "${CMD}"
