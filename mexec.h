#ifndef MEXEC_H
#define MEXEC_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#define MAX_LINE_LENGHT 1024
#define READ_END 0
#define WRITE_END 1
  
char **readStream(void *stream, int *argCount);
int executeCommands(char **args, int argCount);
void waitForChildren(pid_t childPids[], int argCount, char **args);
   
#endif