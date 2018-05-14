#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evalfunc.h"

int evaluarPipe(char **argv, char **argv1, char **argv2) {
    int indexArg, aux, indexArg2;

    for (indexArg = 0; argv[indexArg] != NULL; indexArg++) {
        aux = strcmp("|", argv[indexArg]);
        if (aux == 0)
            break;
        argv1[indexArg] = (char*) malloc(strlen(argv[indexArg] + 1));
        strcpy(argv1[indexArg], argv[indexArg]);
    }
    argv1[indexArg] = '\0';

    if (argv[indexArg] == NULL)
        return 0;

    indexArg++;
    for (indexArg2 = 0; argv[indexArg] != NULL; indexArg2++) {
        if (argv[indexArg] == NULL)
            break;
        argv2[indexArg2] = (char*) malloc(strlen(argv[indexArg] + 1));
        strcpy(argv2[indexArg2], argv[indexArg]);
        indexArg++;
    }
    argv2[indexArg2] = '\0';
    return 1;
}

int evaluarBackground(char **argv) {
    int i;
    for (i = 0; i < 20; i++) {
        if (argv[i] == NULL)
            break;
    }
    if (!strcmp(argv[i - 1], "&"))
        return 1;
    return 0;
}

int evaluarRedir(char **argv, char *filename) {
    int i;
    for (i = 0; i < 20; i++) {

        if (argv[i] == NULL) {
            return 0;
        }
        if (!strcmp(argv[i], "<")) {
            strcpy(filename, argv[i + 1]);
            argv[i] = NULL;
            argv[i + 1] = NULL;
            return 1;
        }
        if (!strcmp(argv[i], ">")) {
            strcpy(filename, argv[i + 1]);
            argv[i] = NULL;
            argv[i + 1] = NULL;
            return 2;
        }
    }
    return 0;
}

