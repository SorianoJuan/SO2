set datafile separator whitespace

# Ejes
set style line 80 lt rgb "#808080"

# Grid
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey
set grid back linestyle 81
set border 3 back linestyle 80

# Tics
set xtics nomirror
set ytics nomirror

# Tipos de trazado
# ps X setea el tamanio el punto en X
set style line 1 lt rgb "#A00000" lw 2 pt 1 ps 1
set style line 2 lt rgb "#00A000" lw 2 pt 2 ps 1
set style line 3 lt rgb "#5060D0" lw 2 pt 3 ps 1
set style line 4 lt rgb "#00008B" lw 2 pt 4 ps 1
set style line 5 lt rgb "#FF1493" lw 2 pt 5 ps 1
set style line 6 lt rgb "#FF8C00" lw 2 pt 6 ps 1
set style line 7 lt rgb "#00FFFF" lw 2 pt 7 ps 1
set style line 8 lt rgb "#DC143C" lw 2 pt 8 ps 1
set style line 9 lt rgb "#FFD700" lw 2 pt 9 ps 1
set style line 10 lt rgb "#FF8C00" lw 2 pt 10 ps 1
set style line 11 lt rgb "#00CED1" lw 2 pt 11 ps 1
set style line 12 lt rgb "#FF00FF" lw 2 pt 12 ps 1

# Labels
set xlabel "Nº ejecucion"
set ylabel "Tiempo de ej. [s]"
set title "Tiempos de ejecucion para todos los casos"

# Plot
plot "<(sed -n '1,30p' tiempos_procedural.txt)" using 1:2 title 'Tiempos procedural local' w lp ls 1, \
"<(sed -n '1,29p' tiempos_procedural_cluster.txt)" using 1:2 title 'Tiempos procedural cluster' w lp ls 2, \
"tiempos_omp_2.txt" using 1:2 title 'Tiempos openmp local (2T)' w lp ls 9, \
"tiempos_omp_4.txt" using 1:2 title 'Tiempos openmp local (4T)' w lp ls 3, \
"tiempos_omp_8.txt" using 1:2 title 'Tiempos openmp local (8T)' w lp ls 4, \
"tiempos_omp_16.txt" using 1:2 title 'Tiempos openmp local (16T)' w lp ls 5, \
"tiempos_omp_32.txt" using 1:2 title 'Tiempos openmp local (32T)' w lp ls 11, \
"tiempos_omp_2_cluster.txt" using 1:2 title 'Tiempos openmp cluster (2T)' w lp ls 10, \
"tiempos_omp_4_cluster.txt" using 1:2 title 'Tiempos openmp cluster (4T)' w lp ls 6, \
"tiempos_omp_8_cluster.txt" using 1:2 title 'Tiempos openmp cluster (8T)' w lp ls 7, \
"tiempos_omp_16_cluster.txt" using 1:2 title 'Tiempos openmp cluster (16T)' w lp ls 8, \
"tiempos_omp_32_cluster.txt" using 1:2 title 'Tiempos openmp cluster (32T)' w lp ls 12
