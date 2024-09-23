#!/bin/bash

cd ../2H4B-full
NAME=$(basename $0)
NAME=${NAME/.sh/}
for ITEM in $(ls -v ../gghh/data4/); do
    python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
        --use-amp -o embed_dims '([256, 32, 32], [32, 32, 32], [32, 32, 32], [32, 32, 32])' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.1 \
        --batch-size 8192 \
        --gpus 2 --fetch-step 1 --in-memory \
        --data-test ../gghh/data4/${ITEM}/*.root \
        --data-config ../gghh-full/pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
        --model-prefix ${NAME/pred/full} --tensorboard _pred_${ITEM}_${NAME/pred/full} --log-file ../gghh-full/${NAME}_${ITEM}.log \
        --predict-output ../gghh-full/${NAME}_${ITEM}.root \
        </dev/null 2>&1 | tee ../gghh-full/${NAME}_${ITEM}.sh.log &
done
wait
