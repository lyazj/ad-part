#!/bin/bash

python3 $HOME/work/weaver-core-dev/weaver/train.py \
    --use-amp -o embed_dims '[64,256,64]' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.01 \
    --batch-size 512 --start-lr 1e-3 --num-epochs 50 --optimizer ranger \
    --gpus 3 --fetch-step 1 --in-memory \
    --samples-per-epoch $((512 * 1500)) --samples-per-epoch-val $((512 * 375)) \
    --data-train ../sm-md/data4/QCD/*.root ../sm-md/data4/VJets/*.root ../sm-md/data4/TTbar/*.root ../sm-md/data4/H2B/*.root \
    --data-config input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
    --model-prefix hid --tensorboard _hid --log-file train.log \
    </dev/null 2>&1 | tee train.sh.log
