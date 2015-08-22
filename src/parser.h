#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "jobControl.h"

#define TRUE	1
#define	FALSE	0
#define DEFAULT_COMMAND_SIZE	100

#define NONE		-1
#define PIPE		0
#define INPUT_FROM_FILE	1
#define OUTPUT_IN_FILE	2
#define APPEND_IN_FILE	3
#define PIPE_READ	0
#define PIPE_WRITE	1

int getType(char *token);
process *parseCommand(int tokenSize, char **tokens, int *foreground);
int tokenCounter(char *command); 
int readCommand(char ***tokens, char **command);
#endif
