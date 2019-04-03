#include <netinet/in.h> //Constantes y estructuras para IP
#include <stdio.h>      //Los input y output mas comunes
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //Estructuras necesarias para los sockets
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

#define LISTEN_PORT 2019 // Puerto TCP
#define UDP_CLIENT_PORT 2019

#define SNAME 20
#define USER_NR 2

#define BUFF_SIZE 1024

#define RETRY_LIMIT 3

struct Login {
    char name[SNAME];
    char password[SNAME];
};

int verificar(char *user, char *password);
int getTelemetria(char *ipaddr);

int main(void)
{
    int sockfd, sockfd2, clientaddrsize;

    char buffer[BUFF_SIZE], bufferaux[BUFF_SIZE];
    int pid;
    long byteRead = 0;
    char *msg, *keyword;

    struct sockaddr_in serv_addr,
            cli_addr;

    uint16_t portnr;

    sockfd = socket(AF_INET, SOCK_STREAM,0);           /*Crear el socket: AF_INET porque es un socket de internet
                                                        SOCK_STREAM: socket de un flujo continuo de datos como si fuera un
                                                        pipe
                                                        0: para elegir el protocolo apropiado, 0=TCP con SOCK_STREAM*/
    if (sockfd < 0) { // Se produjo un error
        perror("ERROR al abrir el socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    portnr = LISTEN_PORT; // Asignar el numero de puerto
    serv_addr.sin_family =
            AF_INET; /*serv_addr es una estructura del tipo sockaddr_in, el primer
                           campo contiene un codigo del tipo address family siempre se
                           debe asignar =AF_INET*/
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Asignar IP del server a s_addr
    serv_addr.sin_port = htons(portnr); /*sin_port contiene el numero de puerto, antes debo usar
                                 htons para convertir el numero de puerto a un orden de
                                 bytes de network*/

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0) {  // Llamada de sistema que toma como argumentos
        perror("ERROR en bind()");                                            // el file descriptor, la dirección ip casteada del server
        exit(1); // y el tamanio de la IP
    }
    listen(sockfd, 5);

    clientaddrsize = sizeof(cli_addr);

    int autenticado = 0;
    int retryCount = RETRY_LIMIT;
    while (1) {
        sockfd2 = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clientaddrsize); // Bloquea el proceso hasta// que haya una conexion
        if (sockfd2 < 0) {
            perror("ERROR en accept()");
            exit(1);
        }

        pid = fork ();
        if (pid < 0 ) { // ERROR
             perror("Error en fork()");
        } else if (pid == 0) { //Child
            msg = "Conexion con estacion terrestre establecida. Dispone de 3 intentos para autenticarse. Ingrese el usuario y luego la contrasena\n";
            if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                perror("ERROR escribiendo en el socket TCP");
            }
            while (!autenticado) {
                char user[20];
                char password[20];
                //Recibir usuario
                if ((byteRead = recv(sockfd2, buffer, BUFF_SIZE-1, 0)) != 0) {
                    if (byteRead <= 0) {
                        perror ("ERROR leyendo del socket");
                        continue;
                    } else {
                        buffer[byteRead] = '\0';
                    }
                }
                strcpy (user, strtok(buffer, "\n"));

                msg = "Ingrese el password\n";
                if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                    perror("ERROR escribiendo en el socket TCP");
                };

                // Recibir password
                if ((byteRead = recv(sockfd2, buffer, BUFF_SIZE - 1, 0)) != 0) {
                    if (byteRead <= 0) {
                        perror("ERROR leyendo del socket");
                        continue;
                    } else {
                        buffer[byteRead] = '\0';
                    }
                }
                strcpy(password, strtok(buffer, "\n"));
                if (verificar(user, password)) {
                    autenticado = 1;
                    msg = "autenticado";
                    if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                        perror("ERROR escribiendo en el socket TCP");
                    }
                    sleep(0.5);
                } else if (retryCount > 1){ //Intentos validos
                    retryCount --;
                    msg = "Usuario y/o contrasena invalidos (pocos intentos restantes), por favor ingrese el usuario\n";
                    if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                        perror("ERROR escribiendo en el socket TCP");
                    }
                } else { //Intentos agotados
                    printf("Cliente agoto el numero de intentos permitidos de login \n");
                    msg = "Numero de intentos de autenticacion agotados. Cerrando conexion\n";
                    if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                        perror("ERROR escribiendo en el socket TCP");
                    }
                    close(sockfd2);
                    kill(getpid(),SIGINT);
                }

            } //Fin de autenticacion
            while (1) { //Recepcion de comandos
                msg = "@base_terrestre >>> ";
                if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                    perror("ERROR escribiendo en el socket TCP");
                }
                if ((byteRead = recv(sockfd2, buffer, BUFF_SIZE-1, 0)) != 0) {
                    if (byteRead <= 0) {
                        perror ("ERROR leyendo del socket");
                        continue;
                    } else {
                        buffer[byteRead] = '\0';
                        printf("Mensaje del cliente: %s", buffer);
                        strcpy(bufferaux,buffer);
                        keyword = strtok(bufferaux, "\n");
                        if (keyword != NULL) {
                            if (strcmp(keyword, "update firmware.bin") == 0) { //enviar archivo firmware.bin
                                printf("DEBUG: peticion de update de firmware\n");
                            } else if (strcmp(keyword, "start scanning") == 0) { //recibir imagen
                                printf("DEBUG: imagen de satelite\n");
                            } else if (strcmp(keyword, "obtener telemetria") == 0) { //abrir socket UDP para escuchar
                                printf("DEBUG: recibiendo telemetria\n");
                                getTelemetria(inet_ntoa(cli_addr.sin_addr));
                            }
                        }
                    }
                }
            }
        } else { //Father
            printf("Conexion cliente-servidor establecida\n");
            close(sockfd2);
            continue;
        }

    }

}

int verificar(char *user, char *password) {
    /**
       @brief Verifica del lado del server que los datos ingresados por el cliente
       correspondan a un usuario registrado
       @param user el nombre de usuario ingresado
       @param password el password ingresado
    **/
    struct Login users[USER_NR] = {{"alumno", "fcefyn"}, {"admin", "admin"}};
    struct Login query;

    strcpy(query.name, user);
    strcpy(query.password, password);
    for (int i = 0; i < USER_NR; i++) {
        if (strcmp(query.name, users[i].name) == 0 &&
            strcmp(query.password, users[i].password) == 0) {
            return 1;
        }
    }
    return 0;
}

int getTelemetria (char *ipaddr){

    int sockfd, sizeofdest;
    struct sockaddr_in dest_addr;

    char buffer[BUFF_SIZE], bufferaux[BUFF_SIZE];

    char * word = NULL;

    int success = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR en apertura de socket");
        exit(1);
    }
    memset(&dest_addr, 0, sizeof(dest_addr));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_CLIENT_PORT);
    inet_aton(ipaddr, &dest_addr.sin_addr);

    sizeofdest = sizeof(dest_addr);

    char *msg = "udpopen";
    // Enviar un mensaje al cliente para indicarle que el puerto UDP esta siendo escuchado
    if (sendto(sockfd, msg, strlen(msg), 0,
               (struct sockaddr *)&dest_addr, (socklen_t)sizeofdest) < 0) {
        perror("ERROR durante escritura en socket UDP");
        exit(0);
    }

    int finish = 0;

    while (finish == 0) {
        memset(buffer, 0, BUFF_SIZE);

        if (recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&dest_addr,
                     (socklen_t *)&sizeofdest) < 0) {
            perror("ERROR durante lectura en socket UDP");
            exit(0);
        }

        if (!strcmp(buffer, "finishudp")) {
            finish = 1;
        } else {
            printf("Recibido UDP: %s\n" , buffer);
            strcpy(bufferaux,buffer);
            printf("telemetria recibida: \n");
            word = strtok(bufferaux, "|");
            printf("ID del satelite = %s\n", word);
            word = strtok(NULL, "|");
            printf("uptime = %s segundos\n",  word);
            word = strtok(NULL, "|");
            printf("version del software = %s\n", word);
            word = strtok(NULL, "\n");
            printf("freeram = %s bytes\n", word);
        }
    }
    shutdown(sockfd, 2);
    return success;
}