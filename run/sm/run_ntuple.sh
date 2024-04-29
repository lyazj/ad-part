#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $(basename $0) <jetfile>" >&2
    exit 1
fi
JETFILE="$1"

if ! grep -q '\.gz$' <<< "${JETFILE}"; then
    echo "ERROR: expect filename *.gz" >&2
    exit 1
fi

NTUPLE="$(sed 's@\.gz$@_ntuple.root@' <<< "${JETFILE}")"

CMD="../../bin/ntuple '${JETFILE}' '${NTUPLE}'"
echo "${CMD}"
eval "${CMD}"
