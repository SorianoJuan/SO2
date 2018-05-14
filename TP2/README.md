- Para buildear ejecutar make
- En la carpeta build se generaran los ejecutables
- Los scripts en la carpeta build ejecutan 30 veces el archivo correspondiente
- Para plotear utilizar gnuplot con el comando plot 'nombre_de_archivo.gnu". En la carpeta data se encuentran los scripts de ploteo:
    * plot_all.gnu: plotea todos los archivos de tiempos
    * plot_openmp.gnu: plotea solamente los archivos de tiempos de openmp
    * plot_procedural.gnu: plotea solamente los archivos de tiempos procedurales
