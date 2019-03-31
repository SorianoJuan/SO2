#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

void *writeto_socket(void *arg);

struct sockbuff {
    int sockfd;
    char *buffer;
    size_t buffer_size;
};

uint16_t portnr = 2019;
char ipaddrbuff [20] = "127.0.0.1";
char * ipaddr = NULL;

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
                    /*char *msg = "gopez";
                    if (send(sockfd, msg, strlen(msg), 0) < 0) {
                        perror("ERROR enviando");
                    }*/
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

void *writeto_socket(void *arg) {
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
    while (1) {
        fgets(msg, msg_size, stdin);
        strcpy(auxbuffer, msg);
        keyword1 = strtok(auxbuffer, " ");
        keyword2 = strtok(NULL, "\n");
        if (keyword1 !=NULL){
            if (keyword2 != NULL){
                strcpy(keywords, keyword1);
                strcat(keywords, keyword2);
                if (strcmp(keywords, "updatefirmware.bin") == 0) {
                    printf("firmware");
                } else if (strcmp(keywords, "startscanning") == 0) {
                    printf("start scanning");
                } else if (strcmp(keywords, "obtenertelemetria") == 0) {
                    printf("obtenertelemetria");
                }
            }
            if (send(sockfd, msg, strlen(msg), 0) < 0) {
                perror("ERROR enviando");
            }
        }
    }
    return 0;
}