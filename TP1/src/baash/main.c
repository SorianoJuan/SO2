#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <string.h>

#include "evalfunc.h"
#include "archfunc.h"

void ejecPipeline(char* argv1[], char* argv2[], char* folders[]);
int parsearPath(char **folders);
int leerTeclado(char **argv, char *input);

int main() {

    char* argv[20];
    char* argv_aux1[20];
    char* argv_aux2[20];
    int argc;

    int pid;
    int waiting;
    int flag_background;
    int flag_pipe;

    char comando_archivo[256];
    char comando[256];

    char *carpetas[20];

    char hostname[50];
    char* username;

    setvbuf(stdout, NULL, _IONBF, 0);

    gethostname(hostname, 50);
    username = getpwuid(getuid())->pw_name;//Usuario
    parsearPath(carpetas);

    while (1) {
        strcpy(comando, "\n");
        waiting = 0;
        flag_pipe = 0;

        printf("%s@%s:%s$ ", username, hostname, getcwd(NULL, 50));
        fgets(comando, 256, stdin);//Esperar el input del usuario

        if (!strcmp(comando, "\n")) {//Si el input no es null, continuar
            printf("\n");
            continue;
        }


        argc = leerTeclado(argv, comando);

            if (!strcmp(comando, "exit"))//Si se ingresa exit, salir
                return 0;
            if (!strcmp(argv[0], "cd")) {//Si se ingresa cd, cambiar de directorio al argumento que sigue
                chdir(argv[1]);
                continue;
            }

            char filename[50];
            int flag_redirected = 0;
            flag_redirected = evaluarRedir(argv, filename);
            int flag_pipeline = 0;
            flag_pipeline = evaluarPipe(argv, argv_aux1, argv_aux2);

            flag_background = evaluarBackground(argv);
            if (flag_background) {
                argv[argc - 1] = NULL;
            }

        buscarArchivo(argv[0], carpetas, comando_archivo);

            if (comando_archivo[0] == 'E'){
                printf("No se encontro el comando\n");
            }
            else {
                pid = fork();
                if (pid < 0) {
                    perror("Error de creacion de hijo");
                    exit(1);
                } else if (pid == 0) {
                    if (flag_redirected == 2) {
                        aArchivo(filename);
                    } else if (flag_redirected == 1) {
                        freopen(filename, "r", stdin);
                    } else if (flag_pipeline == 1) {
                        ejecPipeline(argv_aux1, argv_aux2, carpetas);
                        flag_pipe = 1;
                    }
                    if (!flag_pipe) {
                        execv(comando_archivo, argv);
                        perror(comando_archivo);
                        exit(1);
                    }
                } else {
                    waiting = -1;
                }
                if (flag_background)
                    waitpid(pid, &waiting, WNOHANG);
                else
                    waitpid(pid, &waiting, 0);
            }

    }
}

int parsearPath(char **folders) {
    int counter;
    char* var = getenv("PATH");

    folders[0] = strtok(var, ":");
    for (counter = 1; counter < 20; counter++) {
        folders[counter] = strtok(NULL, ":");
        if (folders[counter] == NULL)
            break;
    }
    strtok(NULL, ":");
    return counter + 1;
}

int leerTeclado(char **argv, char *input) {
    int counter = 0;

    argv[0] = strtok(input, " \n");//Divide el string input en una serie de tokens delimitados por \n
    for (counter = 1; counter < 20; counter++) {
        argv[counter] = strtok(NULL, " \n");
        if (argv[counter] == NULL)
            break;
    }
    return counter;//Contar args
}

void ejecPipeline(char* argv1[], char* argv2[], char* folders[]) {
    char command[256];

    //Pipe
    int fd[2];
    pipe(fd);

    //Evaluar si es el proceso hijo
    if (fork() == 0){
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);

        buscarArchivo(argv1[0], folders, command);
        execv(command, argv1);
        perror(command);

        exit(1);
    }else{
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);

        buscarArchivo(argv2[0], folders, command);
        execv(command, argv2);
        perror(command);

        exit(1);

    }
}
