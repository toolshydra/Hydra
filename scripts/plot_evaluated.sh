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
set xlabel "Podas Aplicadas"
set ylabel "R"
set title "Razão de configurações avaliadas"
#set key outside bmargin
#set key box
set logscale ycb
set yrange [1e-6:2]

#set format  y "%.2f"
set style data histogram
set style histogram errorbars gap 2 lw 2
set xtics nomirror rotate by -45
set style fill pattern 2 border -1
set boxwidth 0.75
plot "$i" using 2:3:xticlabel(1) title ""
set output
set terminal pop
EOF
done
