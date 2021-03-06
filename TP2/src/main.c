#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

/* nombre del archivo a leer */
#define FILE_NAME "../includes/OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"
#define FILE_OUT_NC "../includes/convolved_out.nc"
#define FILE_OUT_BIN "../includes/convolved_out.bin"
#define FILE_OUT_TIMES "../data/tiempos_omp_32.txt"

/* Lectura de una matriz de 21696 x 21696 */
#define NX 21696
#define NY 21696
#define N_KERNEL 3

void convolve (float *data_in, float kernel[][N_KERNEL], float *data_out);
void write_to_nc(float *data_out);
void write_to_bin(float *data_out);

/**
 * @brief Funcion principal que aloca memoria para la imagen de entrada y la convolucionada de salida. Hace uso de la
 * libreria netcdf para importar la imagen y luego llama a la función convolve para realizar la convolución.
 *
 * @return Siempre retorna 0 y sale del programa
 */
int main()
{
    float *data_out = (float*)calloc((NX-N_KERNEL+1) * (NY-N_KERNEL+1), sizeof(float));
    //float *data_out = (float*)calloc(NX*NY, sizeof(float));
    float *data_in = (float*)calloc(NX*NY, sizeof(float));

    float kernel [N_KERNEL][N_KERNEL] = {{-1, -1, -1},
                                         {-1, 8, -1},
                                         {-1, -1, -1}};

    int ncid, varid, retval;

    size_t start[2] = {0};
    size_t count[2] = {0};

    count[0] = NX;
    count[1] = NY;

    start[0] = 0;
    start[1] = 0;

    printf("Reading NC file into memory\n");

    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);

    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        ERR(retval);

    /* Leemos la matriz. */
    if ((retval = nc_get_vara_float(ncid, varid, start, count, data_in)))
        ERR(retval);

    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
    ERR(retval);

    /* el desarrollo acá */
    printf("Convolving...\n");
    double start_time = omp_get_wtime();
    convolve(data_in, kernel, data_out);
    double time = omp_get_wtime() - start_time;
    printf("Convolve time: %f\n", time);

    free(data_in);

    //write_to_nc(data_out);
    printf("Writing file to binary output\n");
    write_to_bin(data_out);
    free(data_out);

    //Impresion de tiempo en ejecucion
    printf("Writing execution time to file\n");
    FILE *tiemposFile;
    tiemposFile = fopen(FILE_OUT_TIMES,"a");
    fprintf(tiemposFile, "%f\n", time);
    fclose(tiemposFile);

    printf("Done! Freeing resources and exiting\n");

    return 0;
}

/**
 * @brief Realiza la convolucion de data_in * kernel y la almacena en data_out
 * @param data_in Puntero a float que contiene la imagen de entrada
 * @param kernel Arreglo de float con el kernel para la convolucion
 * @param data_out Puntero a float que contiene la imagen de salida
 */
void convolve(float *data_in, float kernel[][N_KERNEL], float *data_out)
{
    #pragma omp parallel for collapse (2)
        for (int fil_img = 0; fil_img < (NX - N_KERNEL + 1); fil_img++) { //Iterar sobre filas de la imagen
            for (int col_img = 0; col_img < (NY - N_KERNEL + 1); col_img++) { //Iterar sobre columnas de la imagen
                for (int fil_kernel = 0; fil_kernel < N_KERNEL; fil_kernel++) { //Iterar sobre filas del kernel
                    for (int col_kernel = 0; col_kernel < N_KERNEL; col_kernel++) { //Iterar sobre columnas del kernel
                        /*if ((fil_img>=(NX-N_KERNEL-2)) && (col_img>NY-N_KERNEL-3)){
                            printf("fil_img: %i, col_img: %i, fil_kernel: %i, col_kernel: %i \n", fil_img, col_img, fil_kernel, col_kernel);
                        }*/
                        data_out[fil_img * (NX - N_KERNEL + 1) + col_img] +=
                                data_in[(fil_img + fil_kernel) * NX + (col_img + col_kernel)] *
                                kernel[fil_kernel][col_kernel];
                    }//col_kernel
                }//fil_kernel
            }//col_img
        }//fil_img
}
/**
 * @brief Exporta la imagen de salida en el archivo FILE_OUT_NB definido como macro
 * @param data_out Puntero a float que contiene la imagen de salida
 */
void write_to_nc(float *data_out)
{
    int ncid, varid, retval;
    size_t start[2] = {0};
    start[0] = 0;
    start[1] = 0;
    size_t count[2] = {0};
    count[0] = NX;
    count[1] = NX;

    if ((retval = nc_open(FILE_OUT_NC, NC_WRITE | NC_SHARE, &ncid)))
        ERR(retval);

    if ((retval = nc_inq_varid(ncid, "CMI", &varid))) {
        ERR(retval);
    }

    if ((retval = nc_put_vara_float(ncid, varid, start, count, data_out))){
        printf("retval:_%i", retval);
        ERR(retval);
    }

    if ((retval = nc_close(ncid))) {
        ERR(retval);
    }
}

/**
 * @brief Guarda la imagen de salida como un archivo binario de floats
 * @param data_out Puntero a float que contiene la imagen de salida
 */
void write_to_bin(float *data_out)
{
    int fd;
    if ((fd = open(FILE_OUT_BIN, O_WRONLY|O_CREAT|O_TRUNC, 0666)) <0)
    {
        printf("Error creando el file\n");
        return;
    }

    if ((write(fd, data_out, (NX-N_KERNEL+1) * (NY-N_KERNEL+1) * sizeof(float)) < 0))
    {
        perror("ERROR escribiendo en el file");
        exit(EXIT_FAILURE);
    }

    close(fd);
}

