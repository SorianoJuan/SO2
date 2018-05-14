#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define CANT_PULSOS 800

struct pulso{
    float I;
    float Q;
};

typedef struct {
    uint16_t validSamples;
    struct pulso pulsos_iq[];
}tabla_pulsos;

void parsearpulsos (int fd, tabla_pulsos* tablas[]);

void calcularmedia (tabla_pulsos* tablas[], float gates [2][CANT_PULSOS][500][2]);

void autocorrelacionar (float gates[2][CANT_PULSOS][500][2], float Rt[2][500]);

int main (void) {

    tabla_pulsos* tablas[CANT_PULSOS];
    float gates[2][CANT_PULSOS][500][2];
    float Rt[2][500];

    int fd;
    double time = 0;

    printf("Procedural:\n");
    printf("Abriendo archivo de pulsos\n");

    if((fd = open("../pulsos.iq", O_RDONLY)) < 0)
    {
        perror("ERROR abriendo archivo\n");
        exit(EXIT_FAILURE);
    }

    //Calculo del tiempo para cargar en memoria
    clock_t bloqueStart = clock();
    parsearpulsos(fd,tablas);
    close(fd);
    clock_t bloqueEnd = clock();
    double tiempoBloque = (double) (bloqueEnd - bloqueStart) /CLOCKS_PER_SEC;
    printf ("Tiempo en levantar bloque a memoria: %f\n",tiempoBloque);

    //Computos
    double begin = omp_get_wtime();
    calcularmedia(tablas, gates);
    autocorrelacionar(gates, Rt);
    double end = omp_get_wtime();

    //Output
    FILE *outputFile;
    outputFile = fopen("../output.bin","w");
    fwrite(&gates, sizeof(gates),1,outputFile);
    fwrite(&Rt, sizeof(Rt),1,outputFile);
    fclose(outputFile);

    //Liberar memoria alocada para tablas
    for (int i = 0; i<CANT_PULSOS; i++){
        free(tablas[i]);
    }

    //Impresion de tiempo en ejecucion
    time = (end-begin);
    FILE *tiemposFile;
    tiemposFile = fopen("../tiempos_procedural.txt","a");
    printf("Tiempo de procesamiento: %f\n",time);
    fprintf(tiemposFile, "%f\n", time);
    fclose(tiemposFile);
    exit (EXIT_SUCCESS);
}

void parsearpulsos (int fd, tabla_pulsos* tablas[]){
/**
 @brief Levanta el archivo pulsos.iq a memoria asignandolo a la tabla de pulsos ordenadamente segun corresponda
 @param fd file descriptor del archivo pulsos.iq
 @param tablas arreglo de punteros a structs donde se almacenan los valores parseados del archivo pulsos.iq
 **/
    uint16_t samples=0;
    int count = 0;
    for (int i=0;i<CANT_PULSOS;i++){
        if ((count = (int) read(fd, &samples, sizeof(uint16_t))) < 0 )
        {
            perror("ERROR leyendo");
            printf ("%i",count);
            exit(EXIT_FAILURE);
        }
        tablas[i] = malloc(sizeof(uint16_t)+sizeof(struct pulso[samples*2]));
        if (!tablas [i])
        {
            perror("ERROR alocando memoria para tabla_pulsos");
            exit(EXIT_FAILURE);
        }
        tablas[i]->validSamples = samples;
        read(fd, &tablas[i]->pulsos_iq[0].I, sizeof(float) * (4 * samples));
    }
}

void calcularmedia (tabla_pulsos* tablas[], float gates[2][CANT_PULSOS][500][2])
{
/**
 @brief Calcula la media de los gates en la tabla de pulsos tablas y los almacena segun corresponda en el arreglo gates
 @param tablas arreglo de punteros a structs donde se almacenan los valores parseados del archivo pulsos.iq
 @param gates arreglo de gates calculado a partir de la media
 **/
    int muestras = 0;
    int offset = 0;
    float suma = 0;
    for (int H_V = 0; H_V < 2; H_V++){
        for (int pulso = 0; pulso < CANT_PULSOS; pulso++){
            muestras = (int) tablas[pulso]->validSamples/500;
            for (int rango = 0; rango < 500; rango++) {
                for (int I_Q = 0; I_Q < 2; I_Q++) {
                    suma = 0;
                    if (H_V == 0) { //Vertical -> V
                        if (I_Q == 0) { //Real -> I
                            for (int i = 0; i < muestras; i++) {
                                suma += tablas[pulso]->pulsos_iq[i + (muestras*rango)].I;
                            }
                            gates[H_V][pulso][rango][I_Q] = suma/muestras;
                        } else { //Imaginario -> Q
                            for (int i = 0; i < muestras; i++) {
                                suma += tablas[pulso]->pulsos_iq[i + (muestras*rango)].Q;
                            }
                            gates[H_V][pulso][rango][I_Q] = suma/muestras;
                        }
                    } else { //Horizontal -> H
                        offset = tablas[pulso]->validSamples;
                        if (I_Q == 0) { //Real -> I
                            for (int i = 0; i < muestras; i++) {
                                suma += tablas[pulso]->pulsos_iq[i + (muestras*rango) + offset].I;
                            }
                            gates[H_V][pulso][rango][I_Q] = suma/muestras;
                        } else { //Imaginario -> Q
                            for (int i = 0; i < muestras; i++) {
                                suma += tablas[pulso]->pulsos_iq[i + (muestras*rango) + offset].Q;
                            }
                            gates[H_V][pulso][rango][I_Q] = suma/muestras;
                        }
                    }
                }
            }
        }
    }
}


void autocorrelacionar (float gates[2][CANT_PULSOS][500][2], float Rt[2][500])
{
/**
 @brief Calcula la autocorrelacion de cada pulso
 @param gates arreglo de gates calculado a partir de la media
 @param Rt arreglo de valores de todos los gates autocorrelacionados por pulso
 **/
    float suma=0;
    float nr1=0;
    float nr2=0;
    for (int H_V = 0; H_V < 2; H_V++) {
        for (int nr_gate = 0; nr_gate < 500 ; nr_gate++){
            suma = 0;
            for (int pulso = 0; pulso < CANT_PULSOS-1; pulso++){
                nr1=sqrtf(gates[H_V][pulso][nr_gate][0]*gates[H_V][pulso][nr_gate][0]+gates[H_V][pulso][nr_gate][1]*gates[H_V][pulso][nr_gate][1]);
                nr2=sqrtf(gates[H_V][pulso+1][nr_gate][0]*gates[H_V][pulso+1][nr_gate][0]+gates[H_V][pulso+1][nr_gate][1]*gates[H_V][pulso+1][nr_gate][1]);
                suma += (nr1*nr2);
            }
            Rt[H_V][nr_gate] = suma/CANT_PULSOS;
        }
    }
}





