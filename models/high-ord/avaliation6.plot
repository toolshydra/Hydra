set term post "CMR10"
set term jpeg
set output "avaliacao6.jpg"
set grid
set xlabel "Applied prunings"
set ylabel "Configurations"
set title "Evaluation by number of configurations"
set key outside bmargin
set key box
set key autotitle columnheader
set yrange [0:4500]

set format  y "%.0f"
set style data histogram
set style histogram cluster gap 1
set xtics nomirror rotate by -45
set style fill solid border -1
set boxwidth 0.75
set style fill solid 0.5
plot	"6tasks4freqs2res-high-ord.txt.dat" using 2:xticlabels(1) title "Total of configurations", \
	"6tasks4freqs2res-high-ord.txt.dat" using 3:xticlabels(1) title "Evaluated configurations", \
	"6tasks4freqs2res-high-ord.txt.dat" using 4:xticlabels(1) title "Feasible configurations"
set output
set terminal pop
