#!/bin/bash

NFILE_PER_GROUP=100
IGROUP=0
IFILE=0
EVTFILES=
JETFILES=
LEPFILES=
PHOFILES=
OUTFILES=
HIDFILES=

for NAME in $(ls rawdata); do
    mkdir -p data4/${NAME}
    for EVTFILE in $(ls rawdata/${NAME}/*_events.gz); do
        JETFILE=${EVTFILE/_events.gz/.gz}
        LEPFILE=${EVTFILE/_events.gz/_leptons.gz}
        PHOFILE=${EVTFILE/_events.gz/_photons.gz}
        OUTFILE=${EVTFILE/_events.gz/_part_out.gz}
        HIDFILE=${EVTFILE/_events.gz/_part_hid.gz}
        #ls ${EVTFILE} ${JETFILE} ${LEPFILE} ${PHOFILE} ${OUTFILE} ${HIDFILE}
        EVTFILES="${EVTFILES} ${EVTFILE}"
        JETFILES="${JETFILES} ${JETFILE}"
        LEPFILES="${LEPFILES} ${LEPFILE}"
        PHOFILES="${PHOFILES} ${PHOFILE}"
        OUTFILES="${OUTFILES} ${OUTFILE}"
        HIDFILES="${HIDFILES} ${HIDFILE}"
        let IFILE+=1
        if [ ${IFILE} = ${NFILE_PER_GROUP} ]; then
            (  ../../bin/ereduce 6 ${EVTFILES} | gzip > data4/${NAME}/${IGROUP}_events.gz    \
            && ../../bin/reduce    ${JETFILES} | gzip > data4/${NAME}/${IGROUP}.gz           \
            && gzip -cd            ${LEPFILES} | gzip > data4/${NAME}/${IGROUP}_leptons.gz   \
            && gzip -cd            ${PHOFILES} | gzip > data4/${NAME}/${IGROUP}_photons.gz   \
            && gzip -cd            ${OUTFILES} | gzip > data4/${NAME}/${IGROUP}_part_out.gz  \
            && gzip -cd            ${HIDFILES} | gzip > data4/${NAME}/${IGROUP}_part_hid.gz &)
            let IGROUP+=1
            IFILE=0
            EVTFILES=
            JETFILES=
            LEPFILES=
            PHOFILES=
            OUTFILES=
            HIDFILES=
        fi
    done
done
