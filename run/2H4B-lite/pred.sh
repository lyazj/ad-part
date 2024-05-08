#!/bin/bash

python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
    --use-amp -o embed_dims '[64,256,64]' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.01 \
    --batch-size 8192 \
    --gpus 1 --fetch-step 1 --in-memory \
    --data-test ../sm/data4/QCD/*.root ../sm/data4/VJets/*.root ../sm/data4/TTbar/*.root ../sm/data4/HH4B/*.root \
    --data-config pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
    --model-prefix lite --tensorboard _pred_lite --log-file pred.log \
    --predict-output pred.root \
    </dev/null 2>&1 | tee pred.sh.log
