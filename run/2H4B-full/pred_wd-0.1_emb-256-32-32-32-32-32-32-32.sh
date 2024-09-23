#!/bin/bash

NAME=$(basename $0)
NAME=${NAME/.sh/}
python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
    --use-amp -o embed_dims '[256, 32], [32, 32], [32, 32], [32, 32]' -o pair_embed_dims '[32,32]' -o num_heads 4 --optimizer-option weight_decay 0.1 \
    --batch-size 8192 \
    --gpus 2 --fetch-step 1 --in-memory \
    --data-test ../sm/data4/QCD/*.root ../sm/data4/VJets/*.root ../sm/data4/TTbar/*.root ../sm/data4/HH4B/*.root \
    --data-config pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
    --model-prefix ${NAME/pred/full} --tensorboard _pred_${NAME/pred/full} --log-file ${NAME}.log \
    --predict-output ${NAME}.root \
    </dev/null 2>&1 | tee ${NAME}.sh.log
