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

# Labels
set xlabel "Nº ejecucion"
set ylabel "Tiempo de ej. [s]"
set title "Tiempos de ejecucion procedural"

# Plot
plot "tiempos_procedural.txt" using 1:2 title 'Tiempos procedural local' w lp ls 1, \
"tiempos_procedural_pulqui.txt" using 1:2 title 'Tiempos procedural pulqui' w lp ls 2
