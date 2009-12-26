#!/bin/bash

if [ -z "$GNUPLOT" ] ; then
	GNUPLOT=gnuplot
fi

for i in $@ ; do
$GNUPLOT << EOF
set term post "CMR10"
set term jpeg
set output "$i.jpg"
set grid
set xlabel "Applied prunings"
set ylabel "Evaluated / Total"
set title "Number of evaluated configurations"
set key outside bmargin
set key box
set yrange [0:1.1]

set format  y "%.2f"
set style data histogram
set style histogram errorbars gap 2 lw 2
set xtics nomirror rotate by -45
set style fill pattern 2 border -1
set boxwidth 0.75
plot "$i" using 2:3:xticlabel(1) title "Evaluated configurations"
set output
set terminal pop
EOF
done
