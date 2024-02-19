#!/bin/bash

if [ $# != 1 ]; then
    echo "usage: $(basename $0) <jetfile>" >&2
    exit 1
fi
JETFILE="$1"
if ! grep -q '.\gz$' <<< "${JETFILE}"; then
    echo "ERROR: expect filename *.gz" >&2
    exit 1
fi

PARTFILE="$(sed 's@\.gz$@_part.gz@' <<< "${JETFILE}")"
LOGFILE="$(sed 's@\.gz$@_part.log@' <<< "${JETFILE}")"

CMD="../../bin/part '${JETFILE}' '${PARTFILE}' &> '${LOGFILE}'"
echo "${CMD}"
eval "${CMD}"
