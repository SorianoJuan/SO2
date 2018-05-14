#ifndef EVALFUNC_H
#define EVALFUNC_H

#include <stdio.h>

int evaluarPipe(char **argv, char **argv1, char **argv2);
int evaluarBackground(char **argv);
int evaluarRedir(char **argv, char *filename);

#endif //EVALFUNC_H
