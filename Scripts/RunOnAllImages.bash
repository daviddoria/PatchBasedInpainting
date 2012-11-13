#!/bin/bash

# This script provides an easy way to inpaint many images automatically. Pass the script the inpainting executable to run,
# a directory containing files with the *.mask and *.png image pairs, and any arguments that the executable requires (minus the
# last argument, which is assumed to be the output filename and is constructed by this script.

# For example, if a normal call is:
# ./ClassicalImageInpainting image.png image.mask 15 image_inpainted.png
# then the call to this script would be
# bash RunOnAllImages.bash ~/build/Projects/PatchBasedInpainting/ClassicalImageInpainting /media/portable/Inpainting/pictures/small 15

# CVPR Paper timings:
# SSD:
# time bash /media/portable/Projects/PatchBasedInpaintingDevelop/Scripts/RunOnAllImages.bash ~/build/Projects/PatchBasedInpainting/ClassicalImageInpainting /media/portable/Data/Inpainting/pictures/small/ 15
# GMH:
# time bash /media/portable/Projects/PatchBasedInpaintingDevelop/Scripts/RunOnAllImages.bash ~/build/Projects/PatchBasedInpainting/InpaintingGMH /media/portable/Data/Inpainting/pictures/small/ 15 100
if [ $# -lt 2 ]; then #-lt is "less than"
  echo "Required parameters: Executable imageDirectory";
  exit;
fi

Executable=$1;
echo "Executable $Executable";

ImageDirectory=$2;
echo "ImageDirectory $ImageDirectory";

# eat the first two arguments, so the rest of the arguments can be passed to the executable
shift
shift

for mask in ${ImageDirectory}/*.mask; do
  echo "Processing $mask...";
  ImagePrefix=`basename $mask .mask`;
  echo "ImagePrefix $ImagePrefix";
  time ${Executable} ${ImageDirectory}/${ImagePrefix}.png $mask "$@" ${ImagePrefix}_inpainted.png;
done;
