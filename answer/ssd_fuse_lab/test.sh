#!/bin/bash

SSD_FILE="/tmp/ssd/ssd_file"
GOLDEN="/tmp/ssd_file_golden"
TEMP="/tmp/temp"
touch ${GOLDEN}
truncate -s 0 ${SSD_FILE}
truncate -s 0 ${GOLDEN}

rand(){
    min=$1
    max=$(($2-$min))
    num=$(cat /dev/urandom | head -n 10 | cksum | awk -F ' ' '{print $1}')
    echo $(($num%$max))
}

case "$1" in
    "test1") 
        cat /dev/urandom | tr -dc '[:alpha:][:digit:]' | head -c 51200 | tee ${SSD_FILE} > ${GOLDEN} 2> /dev/null
        ;;
    "test2")
        cat /dev/urandom | tr -dc '[:alpha:][:digit:]' | head -c 51200 | tee ${SSD_FILE} > ${GOLDEN} 2> /dev/null
        cat /dev/urandom | tr -dc '[:alpha:][:digit:]' | head -c 11264 > ${TEMP}
        for i in $(seq 0 9)
        do
            dd if=${TEMP} skip=$(($i*1024)) of=${GOLDEN} iflag=skip_bytes oflag=seek_bytes seek=$(($i*5120)) bs=1024 count=1 conv=notrunc 2> /dev/null
            dd if=${TEMP} skip=$(($i*1024)) of=${SSD_FILE} iflag=skip_bytes oflag=seek_bytes seek=$(($i*5120)) bs=1024 count=1 conv=notrunc 2> /dev/null
        done
        dd if=${TEMP} skip=10240 of=${GOLDEN} iflag=skip_bytes oflag=seek_bytes seek=0 bs=1024 count=1 conv=notrunc 2> /dev/null
        dd if=${TEMP} skip=10240 of=${SSD_FILE} iflag=skip_bytes oflag=seek_bytes seek=0 bs=1024 count=1 conv=notrunc 2> /dev/null
        ;;
    *)
        printf "Usage: sh test.sh test_pattern\n"
        printf "\n"
        printf "test_pattern\n"
        printf "test1: Sequential write whole SSD size(51200bytes)\n"
        printf "       test basic SSD read & write\n"
        printf "test2:\n"
        printf "       1: Sequential write whole SSD size(51200bytes)\n"
        printf "       2: Override 0, 1, 10, 11, 20, 21, 30, 31, 40, 41, 50, 51, 60, 61, 70, 71, 80, 81, 90, 91 page \n"
        printf "       2: Override 0, 1 page \n"
        printf "       test GC's result\n"
        return 
        ;;
esac

# check
diff ${GOLDEN} ${SSD_FILE}
if [ $? -eq 0 ]
then
    echo "success!"
else
    echo "fail!"
fi

echo "WA:"
./ssd_fuse_dut /tmp/ssd/ssd_file W
rm -rf ${TEMP} ${GOLDEN}