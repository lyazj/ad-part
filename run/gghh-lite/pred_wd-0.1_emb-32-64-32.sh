#!/bin/bash

cd ../2H4B-lite
NAME=$(basename $0)
NAME=${NAME/.sh/}
for ITEM in $(ls -v ../gghh/data4/); do
    python3 $HOME/work/weaver-core-dev/weaver/train.py --predict \
        --use-amp -o embed_dims '[32,64,32]' -o pair_embed_dims '[32,32,32]' -o num_heads 4 --optimizer-option weight_decay 0.1 \
        --batch-size 8192 \
        --gpus 1 --fetch-step 1 --in-memory \
        --data-test ../gghh/data4/${ITEM}/*.root \
        --data-config ../gghh-lite/pred_input.yaml --network-config example_ParticleTransformer2023EvtClassifier.py \
        --model-prefix ${NAME/pred/lite} --tensorboard _pred_${ITEM}_${NAME/pred/lite} --log-file ../gghh-lite/${NAME}_${ITEM}.log \
        --predict-output ../gghh-lite/${NAME}_${ITEM}.root \
        </dev/null 2>&1 | tee ../gghh-lite/${NAME}_${ITEM}.sh.log &
done
wait
