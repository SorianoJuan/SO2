#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h> //Los input y output mas comunes
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024

void *writeto_socket(void *arg);

struct sockbuff {
        int sockfd;
        char *buffer;
        size_t buffer_size;
};

uint16_t portnr;
char ipaddrbuff [20];
char * ipaddr = NULL;

int UDPcom(uint16_t UDPport, char *ipaddr, char *filename);

int main(void) {
        
        int sockfd;
        long byteRead = 0;
        struct sockaddr_in dest_addr;
        
        char buffer[BUFF_SIZE];
        
        char *keyword = NULL, *usuario = NULL, *portchar = NULL;
        
        char output[BUFF_SIZE];
        
        struct sockbuff write_buffer;
        
        setvbuf(stdout, NULL, _IONBF, 0);
        pthread_t write_t;
        
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) { // Se produjo un error
                perror("ERROR al abrir el socket");
                exit(1);
        }
        
        memset(&dest_addr, 0, sizeof(dest_addr));
        
        memset(buffer, 0, sizeof(buffer));
        
        // Parseo de expresion para conexion
        int valid = 0;
        while (!valid) {
                printf("Ingrese connect usuario@numero_ip:nr_puerto\n");
                printf(">>");
                fgets(buffer, BUFF_SIZE, stdin);
                keyword = strtok(buffer, " ");
                usuario = strtok(NULL, "@");
                ipaddr = strtok(NULL, ":");
                strcpy(ipaddrbuff,ipaddr);
                ipaddr = (char *) & ipaddrbuff;
                portchar = strtok(NULL, "\n");
                
                if (keyword == NULL || usuario == NULL || ipaddr == NULL ||
                    portchar == NULL) {
                        printf("Expresion invalida, ingrese nuevamente\n");
                } else {
                        valid = 1;
                }
        }
        
        // Si la expresion es valida se crea el socket
        portnr = (uint16_t)strtol(portchar, (char **)NULL, 10);
        
        dest_addr.sin_family = AF_INET;
        inet_aton(ipaddr, &dest_addr.sin_addr);
        dest_addr.sin_port = htons(portnr);
        
        // Conexion
        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
                perror("ERROR conectando al socket");
                exit(1);
        }
        printf("Conexion establecida\n");
        
        if (strcmp(keyword, "connect") == 0) {
                strcpy(buffer, usuario);
        }
        
        // Autenticacion: mientras no se reciba desde el server que la autenticacion
        // fue exitosa, se repite la autenticacion
        int autenticacion = 0;
        while (!autenticacion) {
                
                if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
                        perror("ERROR escribiendo en el socket");
                }
                
                if ((byteRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) != 0) {
                        if (byteRead <= 0) {
                                perror("ERROR leyendo del socket");
                                continue;
                        } else {
                                buffer[byteRead] = '\0';
                        }
                }
                
                keyword = strtok(buffer, ":"); // Verificar autenticado
                portchar = strtok(NULL, "\n"); // Tomar puerto UDP para la conexion
                if (keyword != NULL && portchar != NULL) {
                        portnr = (uint16_t)strtol(portchar, (char **)NULL, 10);
                }
                if (keyword != NULL && strcmp(keyword, "autenticado") == 0) {
                        autenticacion = 1;
                        memset(buffer, 0, sizeof(buffer));
                        printf("autenticacion exitosa\n");
                } else {
                        printf("%s", buffer);
                        printf(">>");
                        fgets(buffer, BUFF_SIZE, stdin);
                }
        }
        
        // Thread que toma entrada de teclado y escribe en el socket
        write_buffer.sockfd = sockfd;
        write_buffer.buffer = output;
        write_buffer.buffer_size = sizeof(output);
        pthread_create(&write_t, NULL, writeto_socket, &write_buffer);
        
        // Prompt
        while ((byteRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) != 0) {
                if (byteRead <= 0) {
                        perror("ERROR leyendo del socket");
                        continue;
                } else {
                        buffer[byteRead] = '\0';
                        printf("%s", buffer);
                }
        }
        pthread_cancel(write_t);
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
        char *keyword;
        fflush(stdin);
        while (1) {
                fgets(msg, msg_size, stdin);
                strcpy(auxbuffer, msg);
                keyword = strtok(auxbuffer, " ");
                if (keyword != NULL &&
                    strcmp(keyword, "descarga") == 0) { // Si es una descarga
                        keyword = strtok(NULL, "\n");
                        if (UDPcom(portnr, ipaddr, keyword) == 0) {
                                printf("Descarga exitosa!\n");
                        } else {
                                printf("ERROR Descarga Fallida\n");
                        }
                } else if (send(sockfd, msg, strlen(msg), 0) < 0) {
                        perror("ERROR enviando");
                        return (void *)-1;
                }
        }
        return 0;
}

int UDPcom(uint16_t UDPport, char *ipaddr, char *filename) {
        /**
           @brief genera una peticion de descarga de un archivo al servidor por socket
           UDP
           @param UDPport es el puerto que el servidor le asigno al cliente
           @param ipaddr es la direccion ip
           @param filename es el nombre del archivo que se quiere descargar
        **/
        int sockfd, sizeofdest;
        struct sockaddr_in dest_addr;
        
        char buffer[BUFF_SIZE];
        char filenameaux[255];
        
        int success = 0;

        char * token, *last;
        
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
                perror("ERROR en apertura de socket");
                exit(1);
        }
        memset(&dest_addr, 0, sizeof(dest_addr));
        
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDPport);
        inet_aton(ipaddr, &dest_addr.sin_addr);
        
        sizeofdest = sizeof(dest_addr);

        last=filename;  //Ultimo miembro no NULL del path
        strcpy(filenameaux,filename);
        token = strtok(filenameaux,"/");

        while (token!=NULL){
            last = token;
            token = strtok(NULL,"/");
        }

        FILE *incomingFile;
        incomingFile = fopen(last, "w");    //Solo el nombre del archivo
        
        // Enviar nombre del archivo con el path completo
        if (sendto(sockfd, filename, strlen(filename), 0,
                   (struct sockaddr *)&dest_addr, (socklen_t)sizeofdest) < 0) {
                perror("ERROR durante escritura en socket UDP");
                exit(0);
        }
        
        int finish = 0;
        
        // Mientras no se reciba un final de comunicacion, se sigue recibiendo el
        // archivo
        while (finish == 0) {
                memset(buffer, 0, BUFF_SIZE);
                
                if (recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&dest_addr,
                             (socklen_t *)&sizeofdest) < 0) {
                        perror("ERROR durante lectura en socket UDP");
                        exit(0);
                }
                
                if (!strcmp(buffer, "finishudp")) {
                        finish = 1;
                        success = 0;
                } else if (strcmp(buffer, "nofileudp") == 0) {
                        finish = 1;
                        printf("No existe tal archivo. \n");
                        success = 1;
                } else {
                        fprintf(incomingFile, "%s", buffer);
                }
        }
        fclose(incomingFile);
        shutdown(sockfd, 2);
        return success;
}
