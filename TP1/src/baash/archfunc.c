#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "archfunc.h"

void buscarArchivo(char *file, char **carpetas, char *commandFile){
    char commandPath[50];
    char dir_actual[50] = "";
    char* archivo;
    int resultado;

    strcpy(commandPath, file);

    if ((file[0] == '.' && file[1] == '.' && file[2] == '/') || file[0] == '/') {//../ o /
        char *dir;
        char *dir_siguiente;
        int listo = 0;

        if (file[0] == '/') {
            dir_actual[0] = '/';
        }
        dir = strtok(file, "/");
        dir_siguiente = strtok(NULL, "/");

        if (dir_siguiente != NULL){
        strcat(dir_actual, dir);
        }
        else {
            dir_siguiente = dir;
            listo = 1;
        }

        while ((dir_siguiente != NULL) && !listo) {
            dir = dir_siguiente;
            dir_siguiente = strtok(NULL, "/");
            strcat(dir_actual, "/");
            if (dir_siguiente != NULL) {
                strcat(dir_actual, dir);
            }
        }
        archivo = dir;

    }else if (file[0] == '.' && file[1] == '/') {
        getcwd(dir_actual, 50);
        strcat(dir_actual, "/");
        archivo = strtok(NULL, "/");

    }else{
        int i;
        char aux[50];
        for (i = 0; i < 20; i++) {
            if (carpetas[i] == NULL)
                break;

            strcpy(aux, carpetas[i]);
            strcat(aux, "/");
            strcat(aux, file);
            resultado = access(aux, F_OK);

            if (!resultado) {
                strcpy(commandFile, aux);
                return;
            }
        }
        commandFile[0] = 'E';
        return;
    }

    strcat(dir_actual, archivo);
    resultado = access(dir_actual, F_OK);
    if (!resultado)
        strcpy(commandFile, dir_actual);
    else
        commandFile[0] = 'E';
}

void aArchivo(char *archivo) {
    int file;

    file = open(archivo, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if (file < 0) {
        perror("open");
        exit(1);
    }
    close(STDOUT_FILENO);
    if (dup(file) < 0) {
        perror("dup");
        exit(1);
    }
    close(file);
}

void deArchivo(char filename[]) {
    int file;

    close(STDIN_FILENO);
    file = open(filename, O_RDONLY, S_IWUSR | S_IRUSR);
    if (file < 0) {
        perror("open");
        exit(1);
    }
    if (dup(file) < 0) {
        perror("dup");
        exit(1);
    }
    close(file);
}
