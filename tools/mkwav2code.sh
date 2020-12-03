#!/bin/bash
OFN=gen/waves.h
DT=`date`
cat <<EOT > $OFN
/*
 * Automatically generated file
 * Date: $DT
 * Do not edit.
 */
#include <inttypes.h>
struct WAVE_RESOURCE {
  int32_t code;
  void *data;
  uint16_t size;
};

EOT

for filename in wav/*.wav; do
    xxd -i $filename >> $OFN
    #SA=$(xxd -i $filename)
    #echo $SA >> $OFN
done

sed -i 's/unsigned\ int /const\ unsigned\ int /g' $OFN

echo >> $OFN
echo "struct WAVE_RESOURCE RES_WAVE[] = {" >> $OFN

for filename in wav/*.wav; do
    CODE=$(basename "$filename" .wav)
    NAME=CODE
    # special cases
    NA=wav\_$CODE\_wav
    SZ=wav\_$CODE\_wav_len
    case $CODE in
    "minus")
        CODE=-1
        ;;
    "dot")
        CODE=-2
        ;;
    "silence")
        CODE=-3
        ;;
    esac
    echo "{$CODE, $NA, $SZ}," >> $OFN
done
echo "{-4, 0, 0}" >> $OFN
echo "};" >> $OFN
