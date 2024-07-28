#!/bin/bash

cd ../2H4B-raw
for ITEM in $(ls -v ../gghh/data4/); do
    python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
        --use-amp -o embed_dims '[64,256,64]' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.01 \
        --batch-size 8192 \
        --gpus 0 --fetch-step 1 --in-memory \
        --data-test ../gghh/data4/${ITEM}/*.root \
        --data-config ../gghh-raw/pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
        --model-prefix raw --tensorboard _pred_${ITEM}_raw --log-file ../gghh-raw/pred_${ITEM}.log \
        --predict-output ../gghh-raw/pred_${ITEM}.root \
        </dev/null 2>&1 | tee ../gghh-raw/pred_${ITEM}.sh.log &
done
wait
