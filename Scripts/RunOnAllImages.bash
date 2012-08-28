#!/bin/bash

if [ $# -lt 2 ]; then #-lt is "less than"
  echo "Required parameter: imageDirectory patchRadius";
  exit;
fi

ImageDirectory=$1;
echo "ImageDirectory $ImageDirectory";

PatchRadius=$2;
echo "PatchRadius $PatchRadius";

for i in ${ImageDirectory}/*.mask; do
  echo "Processing $i...";
  ImagePrefix=`basename $i .mask`;
  echo "ImagePrefix $ImagePrefix";
  ~/build/Projects/PatchBasedInpainting/ClassicalImageInpainting ${ImageDirectory}/${ImagePrefix}.png $i $PatchRadius ${ImagePrefix}_filled.png;
done;
