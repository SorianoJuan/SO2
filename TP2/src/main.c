#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

/* nombre del archivo a leer */
#define FILE_NAME "../includes/OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"

/* Lectura de una matriz de 21696 x 21696 */
#define NX 21696
#define NY 21696
#define N_KERNEL

int kernel[N_KERNEL][N_KERNEL] = {{-1, -1, -1},
                                  {-1, 8, -1},
                                  {-1, -1, -1}};

float data_in[NX][NY];
float data_out[NX][NY];

void convolute (float array[][], float result[][]);

int main()
{
    int ncid, varid;
    int retval;

    for (int i=0; i<NX; i++){
        for (int j=0; j<NY; j++){
            data_out[i][j] = 0;
        }
    }

    //float ** data_in;

    //data_in = calloc(NX*NY, sizeof(float *));

    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);

    /* Obtenemos elvarID de la variable CMI. */
    if ((retval = nc_inq_varid(ncid, "CMI", &varid)))
        ERR(retval);

    /* Leemos la matriz. */
    if ((retval = nc_get_var_float(ncid, varid, &data_in[0][0])))
        ERR(retval);

    /* el desarrollo acá */
    convolute(data_in);


    /* Se cierra el archivo y liberan los recursos*/
    if ((retval = nc_close(ncid)))
        ERR(retval);

    return 0;
}

void convolute (float input[][], float result[][])
{
    for (int fil_img=0; fil_img<NX; fil_img++){ //Iterar sobre filas de la imagen
        for (int col_img=0; col_img<NY; col_img++){ //Iterar sobre columnas de la imagen
            for (int fil_kernel=0; fil_kernel<N_KERNEL; fil_kernel++){ //Iterar sobre filas del kernel
                for (int col_kernel=0; col_kernel<N_KERNEL; col_kernel++){ //Iterar sobre columnas del kernel

                    int fil_img_aux = fil_img + fil_kernel;
                    int col_img_aux = col_img + col_kernel;

                    data_out[fil_img][col_img] = input[fil_img_aux][col_img_aux] * kernel[fil_kernel][col_kernel];
                }//col_kernel
            }//fil_kernel
        }//col_img
    }//fil_img

}

