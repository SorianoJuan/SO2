#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define ID 1
#define FIRMWARE_FILE "firmware.bin"

void *writeto_socket(void *arg);
void sendTelemetria(void);

struct sockbuff {
    int sockfd;
    char *buffer;
    size_t buffer_size;
};

uint16_t portnr = 2019;
char ipaddrbuff [20] = "127.0.0.1";
char * ipaddr = NULL;
char firmware_version [20] = "1.0.0";

int main(void)
{
    int sockfd;
    long byteRead = 0;
    struct sockaddr_in dest_addr;
    char buffer[BUFF_SIZE],auxbuf[BUFF_SIZE], output [BUFF_SIZE];

    struct sockbuff write_buffer;

    setvbuf(stdout, NULL, _IONBF, 0);
    pthread_t write_t;

    char *keyword = NULL;

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(buffer, 0, sizeof(buffer));

    //version de firmware
    FILE *firmware_file;
    if((firmware_file = fopen(FIRMWARE_FILE, "r"))){
        printf("DEBUG: non-stock firmware encontrado, cargando...\n");
        strcpy(firmware_version,"new firmware");
        fclose(firmware_file);
    } else {
        printf("DEBUG: cargando firmware stock...\n");
        strcpy(firmware_version,"1.0.0");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { // Se produjo un error
        perror("ERROR al abrir el socket");
        exit(1);
    }

    ipaddr = (char *) &ipaddrbuff;


    //Creacion del socket
    dest_addr.sin_family = AF_INET;
    inet_aton(ipaddr, &dest_addr.sin_addr);
    dest_addr.sin_port = htons(portnr);

    // Conexion
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("ERROR conectando al socket");
        exit(1);
    }

    // Thread que toma entrada de teclado y escribe en el socket
    write_buffer.sockfd = sockfd;
    write_buffer.buffer = output;
    write_buffer.buffer_size = sizeof(output);
    pthread_create(&write_t, NULL, writeto_socket, &write_buffer);

    // Prompt
    int autenticacion = 0;
    while ((byteRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) != 0) {
        if (byteRead <= 0) {
            perror("ERROR leyendo del socket");
            continue;
        } else {
            buffer[byteRead] = '\0';
            if (!autenticacion){
                strcpy(auxbuf,buffer);
                keyword = strtok(auxbuf, " ");
                if (keyword != NULL && strcmp(keyword, "autenticado") == 0) { //autenticacion exitosa
                    autenticacion = 1 ;
                } else { //local prompt
                    printf("%s", buffer);
                    printf("@satelite >> ");
                }
            } else {
                printf("%s", buffer);
                memset(buffer, 0, sizeof(buffer));
            }
        }
    }
    pthread_cancel(write_t);
    exit (0);
}

void *writeto_socket(void *arg)
{
    /**
       @brief toma los datos ingresados por el teclado y parsea lo ingresado para ver
       si se trata de un comando o una descarga.
       @param arg es un puntero a la direccion en memoria del struct sockbuff
    **/
    int sockfd = ((struct sockbuff *)arg)->sockfd;
    char *msg = ((struct sockbuff *)arg)->buffer;
    size_t msg_size = ((struct sockbuff *)arg)->buffer_size;
    char auxbuffer[BUFF_SIZE];
    char *keyword1, *keyword2, keywords[40];
    fflush(stdin);
    int opcion = 0;
    while (1) {
        opcion = 0;
        fgets(msg, msg_size, stdin);
        strcpy(auxbuffer, msg);
        keyword1 = strtok(auxbuffer, " ");
        keyword2 = strtok(NULL, "\n");
        if (keyword1 !=NULL){
            if (keyword2 != NULL){
                strcpy(keywords, keyword1);
                strcat(keywords, keyword2);
                if (strcmp(keywords, "updatefirmware.bin") == 0) {
                    opcion = 1;
                } else if (strcmp(keywords, "startscanning") == 0) {
                    opcion = 2;
                } else if (strcmp(keywords, "obtenertelemetria") == 0) {
                    opcion = 3;
                }
            }
            if (send(sockfd, msg, strlen(msg), 0) < 0) {
                perror("ERROR enviando");
            }
            switch (opcion)
            {
                case 0:
                    break;

                case 1:
                    printf("DEBUG: firmware\n");
                    break;

                case 2:
                    printf("DEBUG: start scanning\n");
                    break;

                case 3:
                    printf("DEBUG: obtener telemetria\n");
                    sendTelemetria();
                    break;
            }
        }
    }
    return 0;
}

void sendTelemetria(void)
{
    //TODO: ERROR en bind(): Address already in use cuando se quiere enviar dos veces seguidas telemetria
    //Get telemetria
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if (error != 0) {
        printf("Error en telemetria = %d\n", error);
    }
    char telemetria [200];
    sprintf(telemetria, "%d%s%ld%s%s%s%lu", ID, "|", s_info.uptime, "|" , firmware_version, "|", s_info.totalram - s_info.freeram);
    printf("telemetria: %s\n",telemetria);
    printf("ID del satelite = %u\n", ID);
    printf("uptime = %lu segundos\n",  s_info.uptime);
    printf("version del software = %s\n", firmware_version);
    printf("freeram = %lu bytes\n", s_info.totalram - s_info.freeram);

    //Envio por UDP
    int sockfd, sizeofdest;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR al abrir el socket UDP");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnr);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR en bind()");
        exit(1);
    }
    sizeofdest = sizeof(struct sockaddr); // Tamano del dest

    char buffer[BUFF_SIZE];
    printf("Esperando que el servidor abra el socket\n");
    //Esperar recepcion de estado de listo del servidor
    while (strcmp(buffer, "udpopen") != 0) {
        memset(buffer, 0, BUFF_SIZE);
        if (recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &serv_addr,
                     (socklen_t *) &sizeofdest) < 0) {
            perror("ERROR en lectura del socket UDP");
            exit(0);
        }
    }

    //Proceder a enviar telemetria
    if (sendto(sockfd, telemetria, strlen(telemetria), 0,
               (struct sockaddr *) &serv_addr, (socklen_t) sizeofdest) < 0) {
        perror("ERROR escribiendo en el socket UDP");
        exit(1);
    }

    char * finish = "finishudp";
    if (sendto(sockfd, finish, strlen(finish), 0,
               (struct sockaddr *) &serv_addr, (socklen_t) sizeofdest) < 0) {
        perror("ERROR escribiendo en el socket UDP");
        exit(1);
    }
    printf ("DEBUG: telemetria enviada\n");
    shutdown(sockfd, 2);
}