#!/bin/bash

mkdir -p data4
CMD="../../bin/sample4 rawdata data4 10 &> run_sample4.log"
echo "${CMD}"
eval "${CMD}"
