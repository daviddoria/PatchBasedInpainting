#!/bin/bash

if [ $# -lt 1 ]; then #-lt is "less than"
  echo "Required parameter: imageDirectory";
  exit;
fi

ImageDirectory=$1;
echo "ImageDirectory $ImageDirectory";

for patchRadius in 7 15; do
    for knn in 1 10 50 100 1000; do
        for i in ${ImageDirectory}/*.mask; do
          echo "Processing $i...";
          ImagePrefix=`basename $i .mask`;
          echo "ImagePrefix $ImagePrefix";
          ZeroPaddedKNN=`printf %04d ${knn}`
          #Executable=~/build/Projects/PatchBasedInpainting/InpaintingHistogramSort
          Executable=~/temp/InpaintingHistogramSort

          ${Executable} ${ImageDirectory}/${ImagePrefix}.png $i $patchRadius $knn ${ImagePrefix}_filled_${patchRadius}_${ZeroPaddedKNN}.png >> ${ImagePrefix}_filled_${patchRadius}_${ZeroPaddedKNN}.txt;
        done;
    done;
done;
