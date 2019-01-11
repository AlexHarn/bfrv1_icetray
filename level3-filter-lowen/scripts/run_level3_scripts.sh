#!/bin/bash

INDIR=/home/hignight/run122275

for FILE in `ls -1 ${INDIR}`; do

    BASE=`basename ${FILE} .i3.bz2`
    echo ${BASE}
    ./level3_Master_2012.py -i ${INDIR}/${FILE} -g ~/IceCube/gcd_file/IC86_2/Level2_IC86.2012_data_Run00122275_0502_GCD.i3.gz -o ${BASE}_InIcePulses.i3.bz2 --year=12

done

