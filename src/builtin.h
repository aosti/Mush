#ifndef BUILT_IN_H_
#define BUILT_IN_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void cd(int argSize, char **args);

void pwd();

void fg(int argSize, char **args);

int chooseBuiltin(char *commandName);

int executeBuiltIn(int argSize,char *commandName, char **args);

#endif
