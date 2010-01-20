#!/bin/bash

if [ -z "$GNUPLOT" ] ; then
	GNUPLOT=gnuplot
fi

for i in $@ ; do

PLOT="plot"
for j in `seq 1 6` ; do
	S=`expr 3 \* $j`
	PLOT=$PLOT" \"$i\" using $S:$(expr $S + 1):xticlabel(1) title \" `expr $j + 3`\","
done
PLOT=${PLOT%,}
echo $PLOT

GDFONTPATH=/usr/share/fonts/truetype/latex-xft-fonts/ $GNUPLOT << EOF
set term post
set term jpeg  truecolor nocrop enhanced font "cmr10" 18
set output "$i.jpg"
set decimalsign ','
set grid
set xlabel "Podas Aplicadas"
set ylabel "R"
set title "Razão de Configurações Avaliadas"
#set key outside
set key center Left samplen 0.75 horizontal top box title "Quantidade de Tarefas"
set yrange [1e-6:40]
set logscale ycb

#set format  y "%.2f"
set style data histogram
set style histogram clustered errorbars gap 2 lw 2
set xtics nomirror rotate by -45
set style fill solid border -1
#set boxwidth 10
$PLOT
set output
set terminal pop
EOF
done
