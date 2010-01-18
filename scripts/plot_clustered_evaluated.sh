#!/bin/bash

if [ -z "$GNUPLOT" ] ; then
	GNUPLOT=gnuplot
fi

for i in $@ ; do

PLOT="plot"
for j in `seq 1 6` ; do
	S=`expr 3 \* $j`
	PLOT=$PLOT" \"$i\" using $S:$(expr $S + 1):xticlabel(1) title \" `expr $j + 3` tasks \","
done
PLOT=${PLOT%,}
echo $PLOT

$GNUPLOT << EOF
set term post "CMR10"
set term jpeg
set output "$i.jpg"
set decimalsign ','
set grid
set xlabel "Applied prunings"
set ylabel "R"
set title "Evaluated Configurations Ratio"
#set key outside bmargin
set key box
set yrange [1e-6:2]
set logscale ycb

#set format  y "%.2f"
set style data histogram
set style histogram clustered errorbars gap 2 lw 2
set xtics nomirror rotate by -45
set style fill solid border -1
set boxwidth 1
$PLOT
set output
set terminal pop
EOF
done
