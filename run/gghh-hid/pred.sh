#!/bin/bash

cd ../2H4B-hid
for ITEM in $(ls -v ../gghh/data4/); do
    python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
        --use-amp -o embed_dims '[64,256,64]' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.01 \
        --batch-size 8192 \
        --gpus 3 --fetch-step 1 --in-memory \
        --data-test ../gghh/data4/${ITEM}/*.root \
        --data-config ../gghh-hid/pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
        --model-prefix hid --tensorboard _pred_${ITEM}_hid --log-file ../gghh-hid/pred_${ITEM}.log \
        --predict-output ../gghh-hid/pred_${ITEM}.root \
        </dev/null 2>&1 | tee ../gghh-hid/pred_${ITEM}.sh.log &
done
wait
