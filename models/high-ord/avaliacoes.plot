set term post "CMR10"
set term jpeg
set output "avaliacao3.jpg"
set grid
set xlabel "Podas aplicadas"
set ylabel "Configurações"
set title "Avaliação de configurações"
set key outside bmargin
set key box
set key autotitle columnheader
set yrange [0:35]

set format  y "%.0f"
set style data histogram
set style histogram cluster gap 1
set xtics nomirror rotate by -45
set style fill solid border -1
set boxwidth 0.75
set style fill solid 0.5
plot	"3tasks3freqs3res-high-ord.txt.dat" using 2:xticlabels(1) title "Total de configurações", \
	"3tasks3freqs3res-high-ord.txt.dat" using 3:xticlabels(1) title "Configurações avaliadas", \
	"3tasks3freqs3res-high-ord.txt.dat" using 4:xticlabels(1) title "Configurações satisfatórias"
set output
set terminal pop