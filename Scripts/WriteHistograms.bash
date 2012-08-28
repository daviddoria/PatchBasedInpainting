FileNumber=$1

bash /media/portable/Scripts/Histogram/WriteHistogram.bash BestSSDHistogram_$1.txt BestSSDHistogram.png
bash /media/portable/Scripts/Histogram/WriteHistogram.bash TargetHistogram_$1.txt TargetHistogram.png
bash /media/portable/Scripts/Histogram/WriteHistogram.bash BestHistogram_$1.txt BestHistogram.png
gwenview BestSSDHistogram.png TargetHistogram.png &
gwenview BestHistogram.png TargetHistogram.png &
