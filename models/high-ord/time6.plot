set term post "CMR10"
set term jpeg
set output "tempo6.jpg"
set grid
set style data histogram
set style fill solid border -1
set xtics nomirror rotate by -45
set xlabel "Applied prunings"
set ylabel "Time (s)"
set title "Computation time"
set format  y "%.4f"
set yrange [0:0.025]
set key off
#plot	"3tasks3freqs3res-high-ord.txt.dat" using 5:xticlabels(1)
plot	"6tasks4freqs2res-high-ord.txt.dat" using 5:xticlabels(1)
#plot	"12tasks4freqs2res-high-ord.txt.dat" using 5:xticlabels(1)
set output
set terminal pop
