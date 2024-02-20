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

OUTFILE="$(sed 's@\.gz$@_part_out.gz@' <<< "${JETFILE}")"
HIDFILE="$(sed 's@\.gz$@_part_hid.gz@' <<< "${JETFILE}")"
LOGFILE="$(sed 's@\.gz$@_part.log@' <<< "${JETFILE}")"

CMD="../../bin/part '${JETFILE}' '${OUTFILE}' '${HIDFILE}' &> '${LOGFILE}'"
echo "${CMD}"
eval "${CMD}"
