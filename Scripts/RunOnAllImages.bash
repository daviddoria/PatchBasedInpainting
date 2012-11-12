#!/bin/bash

# Example call:
# bash RunOnAllImages.bash ~/build/Projects/PatchBasedInpainting/ClassicalImageInpainting /media/portable/Inpainting/pictures/small 15
if [ $# -lt 3 ]; then #-lt is "less than"
  echo "Required parameter: Executable imageDirectory patchRadius";
  exit;
fi

Executable=$1;
echo "Executable $Executable";

ImageDirectory=$2;
echo "ImageDirectory $ImageDirectory";

PatchRadius=$3;
echo "PatchRadius $PatchRadius";

for i in ${ImageDirectory}/*.mask; do
  echo "Processing $i...";
  ImagePrefix=`basename $i .mask`;
  echo "ImagePrefix $ImagePrefix";
  ${Executable} ${ImageDirectory}/${ImagePrefix}.png $i $PatchRadius ${ImagePrefix}_filled.png;
done;
