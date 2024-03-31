#!/bin/bash

weaver --data-train '../sm/data4/*/*.root' \
    --data-config input.yaml \
    --network-config model.py \
    --model-prefix tiny \
    --gpus 0,1,2,3 --batch-size 8192 --start-lr 1e-3 --num-epochs 50 --optimizer ranger \
    --log train.log
