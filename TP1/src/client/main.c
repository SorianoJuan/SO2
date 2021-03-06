#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define BUFF_SIZE 1024
#define FILE_BUFFER_SIZE 1500
#define ID 1
#define FIRMWARE_FILE "incoming_updated_client"
#define IMAGE_FILE "../2019.jpg"

void sendTelemetria(void);
int sendScan(int sockfd);
int receiveUpdate (int sockfd);

uint16_t portnr = 2019;
char ipaddrbuff [20] = "192.168.1.141";
char * ipaddr = NULL;
char firmware_version [20] = "1.1.0";

/**
 * @brief Funcion principal del satelite que corre en el cliente. Se intenta conectar cada 5 segundos a la base terrestre y espera comandos para realizar acciones.
 *
 * @return No debe retornar la funcion
 */
int main(void) {
    int sockfd;
    long byteRead = 0;
    struct sockaddr_in dest_addr;
    char buffer[BUFF_SIZE], auxbuf[BUFF_SIZE];

    setvbuf(stdout, NULL, _IONBF, 0);

    char *keyword = NULL;

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(buffer, 0, sizeof(buffer));

    printf("DEBUG: Version del firmware: %s\n", firmware_version);

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
    while (connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
        perror("ERROR reintentando conexion en 5 segundos");
        sleep(5);
    }
    printf("Conexion con base terrena exitosa!\n");

    // Prompt
    while ((byteRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) != 0) {
        if (byteRead <= 0) {
            perror("ERROR leyendo del socket");
            continue;
        } else {
            buffer[byteRead] = '\0';
            strcpy(auxbuf, buffer);
            keyword = strtok(auxbuf, "\n");
            if (keyword != NULL) {
                if (strcmp(keyword, "update firmware.bin") == 0) { //enviar archivo firmware.bin
                    printf("DEBUG: peticion de update de firmware\n");
                    receiveUpdate(sockfd);
                } else if (strcmp(keyword, "start scanning") == 0) { //recibir imagen
                    printf("DEBUG: Peticion de imagen de satelite\n");
                    sendScan(sockfd);
                } else if (strcmp(keyword, "obtener telemetria") == 0) { //abrir socket UDP para escuchar
                    printf("DEBUG: Peticion de telemetria\n");
                    sendTelemetria();
                } else {
                    printf("Mensaje de la base terrena: %s", buffer);
                    memset(buffer, 0, sizeof(buffer));
                }
            }
        }
    }
}

/**
 * @brief Envia por UDP al servidor, al puerto 2019 la telemetria recolectada en el satelite.
 *
 * @details La telemetria consiste en el uptime del sistema operativo, la memoria ram libre, el id del satelite y la version del software.
 */
void sendTelemetria(void)
{
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

    const int trueValue = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueValue, sizeof(trueValue));

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
    memset(buffer, 0, BUFF_SIZE);
    printf ("DEBUG: telemetria enviada\n");
    shutdown(sockfd, 2);
    close(sockfd);
}

/**
 * @brief Envia una imagen del escaneo de la tierra por TCP utilizando la conexion abierta previamente.
 * @param sockfd File descriptor del socket abierto.
 * @return int 0: error al abrir el archivo. int 1: se envio exitosamente la imagen.
 */
int sendScan (int sockfd) {
    int imageFilefd;
    struct stat buf;
    if ((imageFilefd = open(IMAGE_FILE, O_RDONLY)) <0)
    {
        printf("No existe la imagen\n");
        return 0;
    }

    int count;
    char sendBuffer[FILE_BUFFER_SIZE];
    fstat(imageFilefd, &buf);
    off_t fileSize = buf.st_size;
    printf("DEBUG: filesize: %li\n", fileSize);
    int32_t bytes = htonl(fileSize);
    //int32_t packages = htonl((fileSize%(FILE_BUFFER_SIZE)) ? fileSize/(FILE_BUFFER_SIZE)+1 : fileSize/(FILE_BUFFER_SIZE));
    char *sendbytes = (char*)&bytes;
    printf("DEBUG: n° de bytes a enviar : %i\n", ntohl(bytes));

    if (send(sockfd, sendbytes, sizeof(bytes), 0) < 0) {
        perror("ERROR enviando");
    }

    while ((count = (int) read(imageFilefd,sendBuffer,FILE_BUFFER_SIZE)) > 0) {
        if (send(sockfd, sendBuffer, count, 0) < 0) {
            perror("ERROR enviando");
        }
        memset(sendBuffer, 0, BUFF_SIZE);
    }
    close(imageFilefd);
    printf("DEBUG: Finalizado envio de scan\n");
    return 1;
}

/**
 * @brief Recibe el update del firmware y realiza un execve(), matando el proceso actual y levantando uno nuevo con el firmware actualizado.
 * @param sockfd File descriptor del socket abierto en TCP.
 * @return int 0: ha ocurrido un error en la apertura del archivo. int 1: no se pudo ejecutar el execve().
 */
int receiveUpdate (int sockfd)
{
    int firmwareFilefd;
    if ((firmwareFilefd = open(FIRMWARE_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0777)) <0)
    {
        printf("Error creando el file\n");
        return 0;
    }
    char recvBuffer[FILE_BUFFER_SIZE];
    long byteRead = 0;

    uint32_t recbytes;
    if ((byteRead = recv(sockfd, &recbytes, 4, 0)) != 0) {
        if (byteRead <= 0) {
            perror ("ERROR leyendo del socket");
        }
    }
    recbytes = ntohl(recbytes);
    printf ("N° de paquetes a recibir: %i\n", recbytes);

    while (recbytes){
        memset(recvBuffer, 0, FILE_BUFFER_SIZE);
        if ((byteRead = recv(sockfd, recvBuffer, FILE_BUFFER_SIZE, 0)) != 0) {
            if (byteRead <= 0) {
                perror ("ERROR leyendo del socket");
                continue;
            }
        }
        if ((write(firmwareFilefd, recvBuffer, (size_t) byteRead) < 0))
        {
            perror("ERROR escribiendo en el file");
            exit(EXIT_FAILURE);
        }
        recbytes -= byteRead;
    }
    close(firmwareFilefd);
    printf("DEBUG: Finalizada la recepcion del update de firmware\n");
    printf("DEBUG: Reiniciando el sistema con el nuevo firmware\n");
    close(sockfd);
    char exec [100] = "./";
    strcat (exec, FIRMWARE_FILE);
    char *argv [] = {FIRMWARE_FILE, NULL};
    if (execv(FIRMWARE_FILE, argv)){
        perror("Error: execv()");
    }
    return 1;
}
