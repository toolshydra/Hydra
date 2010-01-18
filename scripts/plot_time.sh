#!/bin/bash

if [ -z "$GNUPLOT" ] ; then
	GNUPLOT=gnuplot
fi

NO_ARGS=0
E_OPTERROR=85

if [ $# -eq "$NO_ARGS" ]    # Script invoked with no command-line args?
then
  echo "Usage: `basename $0` options (-y)"
  exit $E_OPTERROR          # Exit and explain usage.
                            # Usage: scriptname -options
                            # Note: dash (-) necessary
fi


while getopts "y:" Option
do
  case $Option in
    y     ) YMAX=$OPTARG ;;
    *     ) echo "Unimplemented option chosen." && exit $E_OPTERROR;;   # Default.
  esac
done

shift $(($OPTIND - 1))

if [ -z "$YMAX" ] ; then
	echo "Specify -y N"
	exit $E_OPTERROR
fi

YRANGE="set yrange [10:$YMAX]"

for i in $@ ; do
$GNUPLOT << EOF
set term post "CMR10"
set term jpeg
set output "$i.jpg"
set grid
set logscale ycb
set xlabel "Podas Aplicadas"
set ylabel "Tempo (us)"
set title "Tempo de Computação"
#set key outside bmargin
#set key box
$YRANGE

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
