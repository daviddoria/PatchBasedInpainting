#!/bin/bash

if [ $# -lt 2 ]; then #-lt is "less than"
  echo "Required parameter: imageDirectory patchRadius";
  exit;
fi

ImageDirectory=$1;
echo "ImageDirectory $ImageDirectory";

PatchRadius=$2;
echo "PatchRadius $PatchRadius";

for knn in 1 10 50 1000; do
    for i in ${ImageDirectory}/*.mask; do
      echo "Processing $i...";
      ImagePrefix=`basename $i .mask`;
      echo "ImagePrefix $ImagePrefix";
      ZeroPaddedKNN=`printf %04d ${knn}`
      ~/build/Projects/PatchBasedInpainting/InpaintingHistogramSort ${ImageDirectory}/${ImagePrefix}.png $i $PatchRadius $knn ${ImagePrefix}_filled_${ZeroPaddedKNN}.png >> ${ImagePrefix}_filled_${ZeroPaddedKNN}.txt;
    done;
done;
