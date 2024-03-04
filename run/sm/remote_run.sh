#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $(basename $0) <index>" >&2
    exit 1
fi
ROOTFILE=$(head -$[$1+1] remote_run.txt | tail -1)
OUTDIR="rawdata/$(basename "$(dirname "${ROOTFILE}")")"
JETFILE="${OUTDIR}"/"$(sed 's@\.root$@.gz@' <<< "$(basename "${ROOTFILE}")")"

cd /afs/ihep.ac.cn/users/g/gaoleyun/publicfs/ubuntu
exec ./exec.sh sh -c "cd /afs/ihep.ac.cn/users/g/gaoleyun/publicfs/ubuntu/ad-part/run/sm && ./run_flatten.sh '${ROOTFILE}' '${OUTDIR}' && ./run_part.sh '${JETFILE}'"
