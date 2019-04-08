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
#include <sys/time.h>

#define LISTEN_PORT 2019 // Puerto TCP
#define UDP_CLIENT_PORT 2019
#define SNAME 20
#define USER_NR 2
#define BUFF_SIZE 1024
#define FILE_BUFFER_SIZE 1000000
#define RETRY_LIMIT 3
#define FIRMWARE_FILE "../updated_client"
#define IMAGE_FILE "../incoming_2019.jpg"

struct Login {
    char name[SNAME];
    char password[SNAME];
};

int verificar(char *user, char *password);
int getTelemetria(char *ipaddr);
int getScan(int sockfd);
int sendUpdate(int sockfd);

/**
 * @brief Servidor que simula base terrestre.
 *
 * Se realiza una etapa de autenticacion por parte del usuario en la base de datos del servidor. Si se acepta, Se continua y se acepta conexiones con los satelites. Se ofrece un prompt para ingresar comandos.
 * **/

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


    int autenticado = 0;
    int retryCount = RETRY_LIMIT;
    char user[50];
    char password[50];
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

    //Socket reutilizable por el SO y conseguir el tamaño del socket utilizado
    const int trueValue = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueValue, sizeof(trueValue));
    int socksize;
    unsigned int m = sizeof(socksize);
    getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(void *)&socksize, &m);
    printf("DEBUG: Tama\244o del socket TCP: %i\n", socksize);


    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0) {  // Llamada de sistema que toma como argumentos
        perror("ERROR en bind()");                                            // el file descriptor, la dirección ip casteada del server
        exit(1); // y el tamanio de la IP
    }
    listen(sockfd, 5);

    clientaddrsize = sizeof(cli_addr);

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
                    sleep(0.3);
                    switch (opcion)
                    {
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

                        default:
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

/**
 * @brief Verifica del lado del server que los datos ingresados por el cliente correspondan a un usuario registrado
 * @param user el nombre de usuario ingresado
 * @param password el password ingresado
 * @return int 0: no autenticado, 1:autenticado.
**/

int verificar(char *user, char *password)
{
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

/**
 * @brief Obtiene la imagen (scan) del satelite por medio del socket TCP ya instanciado.
 * @param sockfd2 file descriptor del socket TCP abierto para la recepcion de la imagen
 * @return int 0: no se pudo abrir el archivo para escritura. int 1: se termino la recepcion exitosamente.
 */
int getScan (int sockfd2)
{
    struct timeval start, end;

    int imageFilefd;
    if ((imageFilefd = open(IMAGE_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0666)) <0)
    {
        printf("Error creando el file\n");
        return 0;
    }
    char recvBuffer[FILE_BUFFER_SIZE];
    long byteRead = 0;

    gettimeofday(&start, NULL);
    uint32_t bytesrecv;
    if ((byteRead = recv(sockfd2, &bytesrecv, 4, 0)) != 0) {
        if (byteRead <= 0) {
            perror ("ERROR leyendo del socket");
        }
    }
    bytesrecv = ntohl(bytesrecv);
    printf ("N° de bytes a recibir: %i\n", bytesrecv);
    while (bytesrecv){
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
        bytesrecv -= byteRead;
    }
    gettimeofday(&end,NULL);
    close(imageFilefd);
    printf("DEBUG: Finalizada la recepcion de scan, tiempo total: %f\n", (float)(((end.tv_sec - start.tv_sec)*1000000 +end.tv_usec) - start.tv_usec)/1000000);
    return 1;
}

/**
 * @brief Abre un socket UDP para recibir la telemetria del satelite y la muestra por pantalla.
 * @param ipaddr direccion ip del satelite conectado proveniente de la estructura almacenada de la conexion tcp
 * @return int 1: recepcion exitosa (no asegura que la informacion sea valida). int 0: error en la conexion.
 */
int getTelemetria (char *ipaddr){

    int sockfd, sizeofdest;
    struct sockaddr_in dest_addr;

    char buffer[BUFF_SIZE], bufferaux[BUFF_SIZE];

    char * word = NULL;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR en apertura de socket");
        return 0;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_CLIENT_PORT);
    inet_aton(ipaddr, &dest_addr.sin_addr);

    const int trueValue = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueValue, sizeof(trueValue));

    sizeofdest = sizeof(dest_addr);

    char *msg = "udpopen";
    sleep(2);

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
    memset(buffer, 0, BUFF_SIZE);
    memset(bufferaux, 0, BUFF_SIZE);
    shutdown(sockfd, 2);
    close(sockfd);
    return 1;
}

/**
 * @brief Envia un update de firmware a los satelites que se encuentran escuchando por medio de TCP
 * @param sockfd File descriptor del socket abierto TCP.
 * @return int 1: envio exitoso. int 0: hubo un error en la apertura del archivo para enviar.
 */
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
    int32_t bytes = htonl(fileSize);
    //int32_t packages = htonl((fileSize%(FILE_BUFFER_SIZE)) ? fileSize/(FILE_BUFFER_SIZE)+1 : fileSize/(FILE_BUFFER_SIZE));
    char *sendbytes = (char*)&bytes;
    printf("DEBUG: n° de bytes a enviar : %i\n", ntohl(bytes));

    if (send(sockfd, sendbytes, sizeof(bytes), 0) < 0) {
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