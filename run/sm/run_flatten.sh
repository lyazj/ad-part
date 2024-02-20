#!/bin/bash

if [ $# != 2 ]; then
    echo "usage: $(basename $0) <rootfile> <outdir>" >&2
    exit 1
fi
ROOTFILE="$1"
OUTDIR="$2"

ROOTDIR="$(dirname "${ROOTFILE}")"
ROOTBASE="$(basename "${ROOTFILE}")"
if ! grep -q '\.root$' <<< "${ROOTBASE}"; then
    echo "ERROR: expect filename *.root" >&2
    exit 1
fi

JETFILE="${OUTDIR}"/"$(sed 's@\.root$@.gz@' <<< "${ROOTBASE}")"
LEPFILE="${OUTDIR}"/"$(sed 's@\.root$@_leptons.gz@' <<< "${ROOTBASE}")"
PHOFILE="${OUTDIR}"/"$(sed 's@\.root$@_photons.gz@' <<< "${ROOTBASE}")"
EVTFILE="${OUTDIR}"/"$(sed 's@\.root$@_events.gz@' <<< "${ROOTBASE}")"
LOGFILE="${OUTDIR}"/"$(sed 's@\.root$@_flatten.log@' <<< "${ROOTBASE}")"

mkdir -p "${OUTDIR}"
CMD="../../bin/flatten 0 '${JETFILE}' '${LEPFILE}' '${PHOFILE}' '${EVTFILE}' '${ROOTFILE}' &> '${LOGFILE}'"
echo "${CMD}"
eval "${CMD}"
