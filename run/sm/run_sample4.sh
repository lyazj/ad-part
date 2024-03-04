#!/bin/bash

mkdir -p data
CMD="../../bin/sample4 rawdata data4 1 &> run_sample4.log"
echo "${CMD}"
eval "${CMD}"
