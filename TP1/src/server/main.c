#include <netinet/in.h> //Constantes y estructuras para IP
#include <stdio.h>      //Los input y output mas comunes
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> //Estructuras necesarias para los sockets
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define LISTEN_PORT 2019 // Puerto TCP
#define UDP_CLIENT_PORT 2019
#define SNAME 20
#define USER_NR 2
#define BUFF_SIZE 1024
#define FILE_BUFFER_SIZE 1500
#define RETRY_LIMIT 3
#define FIRMWARE_FILE "../updated_client"
#define IMAGE_FILE "../incoming_2019.jpg"

struct Login {
    char name[SNAME];
    char password[SNAME];
};

struct sockbuff {
    int sockfd;
    char *buffer;
    size_t buffer_size;
};

void *writeto_socket(void *arg);
int verificar(char *user, char *password);
int getTelemetria(char *ipaddr);
int getScan(int sockfd);
int sendUpdate(int sockfd);

int main(void)
{
    int sockfd, sockfd2, clientaddrsize;
    int pid;
    char buffer[BUFF_SIZE], auxbuffer[BUFF_SIZE];
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
    char user[50];
    char password[50];
    while (1) {
        printf("Esperando que se establezca conexion con algun satelite...\n");
        sockfd2 = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clientaddrsize); // Bloquea el proceso hasta// que haya una conexion
        if (sockfd2 < 0) {
            perror("ERROR en accept()");
            exit(1);
        }

        pid = fork ();
        if (pid < 0 ) { // ERROR
             perror("Error en fork()");
        } else if (pid == 0) { //Child
            //Etapa de autenticacion
            printf("Estacion terrestre. Dispone de 3 intentos para autenticarse. Ingrese el usuario y luego la contrasena\n");
            while (!autenticado) {
                printf("Usuario: ");
                fgets(user,sizeof(user),stdin);
                user[strlen(user)-1]='\0';
                printf("Ingrese el password: \n");
                fgets(password,sizeof(password),stdin);
                password[strlen(password)-1]='\0';
                if (verificar(user, password)) {
                    autenticado = 1;
                    printf("Autenticacion exitosa!\n");
                } else if (retryCount > 1){ //Intentos validos
                    retryCount --;
                    printf("Usuario y/o contrasena invalidos, por favor ingrese el usuario\n");
                } else { //Intentos agotados
                    printf("Numero de intentos de autenticacion agotados. Cerrando conexion\n");
                    //close(sockfd2);
                    kill(getpid(),SIGINT);
                    //exit(EXIT_SUCCESS);
                }
            } //Fin de autenticacion
            char *msg=buffer, *keyword1, *keyword2, keywords[40];
            char opcion;
            while (1) {
                opcion = 0;
                printf("@base_terrestre >>> ");
                fgets(msg,sizeof(buffer),stdin);
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
                    if (send(sockfd2, msg, strlen(msg), 0) < 0) {
                        perror("ERROR enviando");
                    }
                    sleep(0.5);
                    switch (opcion)
                    {
                        case 0:
                            break;

                        case 1:
                            printf("DEBUG: firmware\n");
                            sendUpdate(sockfd2);
                            kill(getpid(),SIGINT);
                            break;

                        case 2:
                            printf("DEBUG: start scanning\n");
                            getScan(sockfd2);
                            break;

                        case 3:
                            printf("DEBUG: obtener telemetria\n");
                            getTelemetria(inet_ntoa(cli_addr.sin_addr));
                            break;
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

int getScan (int sockfd2)
{
    int imageFilefd;
    if ((imageFilefd = open(IMAGE_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0666)) <0)
    {
        printf("Error creando el file\n");
        return 0;
    }
    char recvBuffer[FILE_BUFFER_SIZE];
    long byteRead = 0;

    uint32_t npackages;
    if ((byteRead = recv(sockfd2, &npackages, 4, 0)) != 0) {
        if (byteRead <= 0) {
            perror ("ERROR leyendo del socket");
        }
    }
    npackages = ntohl(npackages);
    printf ("N° de paquetes a recibir: %i\n", npackages);
    for (int i=0; i<npackages; i++){
        memset(recvBuffer, 0, FILE_BUFFER_SIZE);
        if ((byteRead = recv(sockfd2, recvBuffer, FILE_BUFFER_SIZE, 0)) != 0) {
            if (byteRead <= 0) {
                perror ("ERROR leyendo del socket");
                continue;
            }
        }
        if ((write(imageFilefd, recvBuffer, (size_t) byteRead) < 0))
        {
            perror("ERROR escribiendo en el file");
            exit(EXIT_FAILURE);
        }
    }
    close(imageFilefd);
    printf("DEBUG: Finalizada la recepcion de scan\n");
    return 1;
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
    sleep(0.5);
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

int sendUpdate(int sockfd){
    int firmwareFilefd;
    struct stat buf;
    if ((firmwareFilefd = open(FIRMWARE_FILE, O_RDONLY)) <0)
    {
        printf("No existe el update de firmware solicitado\n");
        return 0;
    }
    int count;
    char sendBuffer[FILE_BUFFER_SIZE];
    fstat(firmwareFilefd, &buf);
    off_t fileSize = buf.st_size;
    printf("DEBUG: filesize: %li\n", fileSize);
    int32_t packages = htonl((fileSize%(FILE_BUFFER_SIZE)) ? fileSize/(FILE_BUFFER_SIZE)+1 : fileSize/(FILE_BUFFER_SIZE));
    char *npackages = (char*)&packages;
    printf("DEBUG: n° de paquetes a enviar : %i\n", ntohl(packages));

    if (send(sockfd, npackages, 4, 0) < 0) {
        perror("ERROR enviando");
    }

    while ((count = (int) read(firmwareFilefd,sendBuffer,FILE_BUFFER_SIZE)) > 0) {
        if (send(sockfd, sendBuffer, count, 0) < 0) {
            perror("ERROR enviando");
        }
        memset(sendBuffer, 0, BUFF_SIZE);
    }
    close(firmwareFilefd);
    return 1;
}