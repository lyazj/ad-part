#!/bin/bash

export SKIP_QCD=TRUE
export EXHAUST_EVENTS=TRUE

mkdir -p data4
CMD="../../bin/sample4 rawdata data4 10 &> run_sample4.log"
echo "${CMD}"
eval "${CMD}"
