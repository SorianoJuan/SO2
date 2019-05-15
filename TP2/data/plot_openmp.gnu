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

# Labels
set xlabel "Nº ejecucion"
set ylabel "Tiempo de ej. [s]"
set title "Tiempos de ejecucion paralela con openmp"

# Plot
plot "tiempos_openmp_4t.txt" using 1:2 title 'Tiempos openmp local (4T)' w lp ls 1, \
"tiempos_openmp_8t.txt" using 1:2 title 'Tiempos openmp local (8T)' w lp ls 2, \
"tiempos_openmp_16t.txt" using 1:2 title 'Tiempos openmp local (16T)' w lp ls 3, \
"tiempos_openmp_pulqui_4t.txt" using 1:2 title 'Tiempos openmp pulqui (4T)' w lp ls 4, \
"tiempos_openmp_pulqui_8t.txt" using 1:2 title 'Tiempos openmp pulqui (8T)' w lp ls 5, \
"tiempos_openmp_pulqui_16t.txt" using 1:2 title 'Tiempos openmp pulqui (16T)' w lp ls 6

