#!/bin/bash

for JETFILE in $(ls data4/*/*.gz | grep '[0-9]\.gz$'); do
    echo "${JETFILE}"
    echo "----------------------------------------"
    EVTFILE="$(sed 's@\.gz$@_events.gz@' <<< "${JETFILE}")"
    LEPFILE="$(sed 's@\.gz$@_leptons.gz@' <<< "${JETFILE}")"
    PHOFILE="$(sed 's@\.gz$@_photons.gz@' <<< "${JETFILE}")"
    OUTFILE="$(sed 's@\.gz$@_part_out.gz@' <<< "${JETFILE}")"
    HIDFILE="$(sed 's@\.gz$@_part_hid.gz@' <<< "${JETFILE}")"
    ROOTFILE="$(sed 's@\.gz$@.root@' <<< "${JETFILE}")"
    ../../bin/mkroot "${EVTFILE}" "${JETFILE}" "${LEPFILE}" "${PHOFILE}" \
        "${OUTFILE}" "${HIDFILE}" "${ROOTFILE}"
    echo
done
