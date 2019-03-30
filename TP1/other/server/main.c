#include <netinet/in.h> //Constantes y estructuras para IP
#include <stdio.h>      //Los input y output mas comunes
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //Estructuras necesarias para los sockets
#include <unistd.h>

#define LISTEN_PORT 6020 // Puerto TCP

#define SNAME 20
#define USER_NR 2

#define BUFF_SIZE 1024

struct Login {
        char name[SNAME];
        char password[SNAME];
};

int verificar(char *user, char *password);
void UDPcom(uint16_t UDPport);

int main(void) {

        int sockfd, sockfd2, clientaddrsize; // File descriptors, numero de puerto,
        // tamanio del ipaddr del cliente
        char buffer[BUFF_SIZE]; // Se almacenan los datos leidos del socket en el que
        // se escucha
        struct sockaddr_in serv_addr,
                cli_addr; /*Estructura conteniendo una direccion IP, se define en
                            netinet/in.h serv_addr contiene la direccion del server y
                            cli_addr la direccion del cliente*/
        uint16_t portnr;
        char *msg;

        long byteRead = 0;

        int pid, pid2;

        sockfd = socket(
                AF_INET, SOCK_STREAM,
                0);           /*Crear el socket: AF_INET porque es un socket de internet
                                SOCK_STREAM: socket de un flujo continuo de datos como si fuera un
                                pipe           0: para elegir el protocolo apropiado, 0=TCP con SOCK_STREAM*/
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
        serv_addr.sin_port =
                htons(portnr); /*sin_port contiene el numero de puerto, antes debo usar
                                 htons para convertir el numero de puerto a un orden de
                                 bytes de network*/
        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {                     // Llamada de sistema que toma como argumentos
                perror("ERROR en bind()"); // el file descriptor, la dirección ip casteada
                // del server
                exit(1); // y el tamanio de la IP
        }
        listen(sockfd, 5); // Escuchar en el socket. Toma como argumentos el FD y el
        // segundo es la cantidad de conexiones en espera (max 5)

        clientaddrsize = sizeof(cli_addr);

        int autenticado = 0;

        while (1) {
                sockfd2 = accept(sockfd, (struct sockaddr *)&cli_addr,
                                 (socklen_t *)&clientaddrsize); // Bloquea el proceso hasta
                // que haya una conexion
                if (sockfd2 < 0) {
                        perror("ERROR en accept()");
                        exit(1);
                }
                pid = fork();
                if (pid < 0) { // ERROR
                        perror("ERROR en fork()");
                } else if (pid == 0) { // Child
                        // Autenticacion
                        while (!autenticado) {
                                char user[20];
                                char password[20];
                                // Recibir usuario
                                if ((byteRead = recv(sockfd2, buffer, BUFF_SIZE - 1, 0)) != 0) {
                                        if (byteRead <= 0) {
                                                perror("ERROR leyendo del socket");
                                                continue;
                                        } else {
                                                buffer[byteRead] = '\0';
                                        }
                                }
                                strcpy(user, strtok(buffer, "\n"));

                                msg = "Por favor ingrese el password\n";
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
                                        msg = "autenticado:8123";
                                        if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                                                perror("ERROR escribiendo en el socket TCP");
                                        }
                                } else {
                                        msg = "Usuario y/o contrasena invalidos, por favor ingrese el "
                                                "usuario\n";
                                        if (send(sockfd2, (void *)msg, strlen(msg), 0) < 0) {
                                                perror("ERROR escribiendo en el socket TCP");
                                        };
                                }
                        } // Fin autenticacion

                        // Interfaz descarga o baash
                        pid2 = fork();
                        if (pid2 < 0) {
                                perror("ERROR en fork()");
                        } else if (pid2 == 0) { // Hijo2
                                close(sockfd2);
                                UDPcom(8123);
                        } else {
                                // Comienzo de redireccion de STDIN Y STDOUT para baash
                                if (dup2(sockfd2, STDOUT_FILENO) < 0) {
                                        perror("[ERROR en dup2");
                                        exit(EXIT_FAILURE);
                                }
                                if (dup2(sockfd2, STDIN_FILENO) < 0) {
                                        perror("ERROR en dup2");
                                        exit(EXIT_FAILURE);
                                }
                                close(sockfd2);
                                char *argv[2];
                                argv[0] = "./baash";
                                argv[1] = NULL;
                                if (execvp(argv[0], argv)) {
                                        perror("ERROR ejecutando baash");
                                        exit(EXIT_FAILURE);
                                }
                        }
                } else { // Father
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

void UDPcom(uint16_t UDPport) {
        /**
           @brief Realiza una conexion UDP para enviar un archivo de forma no segura al
           cliente
           @param UDPport es el puerto UDP de conexion con el cliente
        **/
        int sockfd, sizeofdest;
        struct sockaddr_in serv_addr;

        char buffer[BUFF_SIZE];
        char *filename;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
                perror("ERROR al abrir el socket UDP");
        }

        memset(&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(UDPport);

        // Bind socket
        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("ERROR en bind()");
                exit(1);
        }
        sizeofdest = sizeof(struct sockaddr); // Tamano del dest

        while (1) {
                memset(buffer, 0, BUFF_SIZE);
                if (recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *) &serv_addr,
                             (socklen_t *) &sizeofdest) < 0) {
                        perror("ERROR en lectura del socket UDP");
                        exit(0);
                }

                // Tomar el nombre del archivo
                filename = strtok(buffer, "\n");
                if (filename == NULL) {
                        continue;
                }
                FILE *outgoingFile;
                outgoingFile = fopen(filename, "r"); // Abrir para lectura con el flag r

                // Si el archivo no se encuentra, avisar que no existe
                if (outgoingFile == NULL) {
                        char *msg = "nofileudp";
                        if (sendto(sockfd, (void *) msg, strlen(msg), 0,
                                   (struct sockaddr *) &serv_addr, (socklen_t) sizeofdest) < 0) {
                                perror("ERROR escribiendo en el socket UDP");
                                exit(1);
                        }
                        continue;
                }
                char outgoingBuffer[BUFF_SIZE];

                // Hasta no encontrar el EOF, seguir iterando
                while (!feof(outgoingFile)) {
                        memset(outgoingBuffer, 0, BUFF_SIZE);
                        fgets(outgoingBuffer, BUFF_SIZE, outgoingFile);
                        if (sendto(sockfd, outgoingBuffer, strlen(outgoingBuffer), 0,
                                   (struct sockaddr *) &serv_addr, (socklen_t) sizeofdest) < 0) {
                                perror("ERROR escribiendo en el socket UDP");
                                exit(1);
                        }
                }
                fclose(outgoingFile);

                // Finalizar el envio del archivo
                char *finish = NULL;
                finish = "finishudp";
                printf("Archivo enviado \n");
                if (sendto(sockfd, (void *) finish, strlen(finish), 0,
                           (struct sockaddr *) &serv_addr, (socklen_t) sizeofdest) < 0) {
                        perror("ERROR escribiendo en el socket UDP");
                        exit(1);
                }
        }
}
