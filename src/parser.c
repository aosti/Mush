#include "parser.h"

int getType(char *token) {

	if(strcmp(token, "<") == 0) return INPUT_FROM_FILE;
	if(strcmp(token, ">") == 0) return OUTPUT_IN_FILE;
	if(strcmp(token, ">>") == 0) return APPEND_IN_FILE; 
	if(strcmp(token, "|") == 0) return PIPE;
	return NONE;
}

void initProcess(process *p) {
	p->next = NULL;
	p->programToExecute = NULL;
	p->args = malloc(sizeof(char*));
        p->pid = 0;
        p->completed = 0;
        p->stopped = 0;
        p->status = 0;
        p->Pipe = FALSE;
        p->RedirectInput = NULL;
        p->RedirectOutput = NULL;
        p->Overwrite = FALSE;
}

process *parseCommand(int tokenSize, char **tokens, int *foreground){
	int tokenAnalyzed = 0, execCounter = 0, argCounter;
	process *p = malloc(sizeof(process));
	process *firstProcess;

	firstProcess = p;
	p->previous = NULL;
	*foreground = TRUE;
	while(tokenAnalyzed < tokenSize) {
		initProcess(p);
		execCounter++;
		argCounter = 1;

		if(getType(tokens[tokenAnalyzed])!= NONE) {
			printf("Error: Invalid command found\n");
			exit(EXIT_FAILURE); // Change this to message 	
		}
		p->programToExecute = tokens[tokenAnalyzed];
		p->args[argCounter - 1] = tokens[tokenAnalyzed];

		tokenAnalyzed++;
		// arguments to be passed to current program
		while(tokenAnalyzed < tokenSize && getType(tokens[tokenAnalyzed]) == NONE) { 
			argCounter++;
			p->args = realloc(p->args, sizeof(char*) * argCounter);
			p->args[argCounter - 1] = tokens[tokenAnalyzed];
			tokenAnalyzed++; 
		}
		argCounter++;
		//Last argument required to be NULL
		p->args = realloc(p->args, sizeof(char*) * argCounter);
		p->args[argCounter - 1] = NULL;
	
		// analyzes redirection and pipes found on command line
		if(tokenAnalyzed < tokenSize) {
			int tokenType = getType(tokens[tokenAnalyzed]);

			// if < is present
			if(tokenType == INPUT_FROM_FILE) {
				tokenAnalyzed++;
				// pensar se vai tratar erros aqui ou não
				p->RedirectInput = tokens[tokenAnalyzed];
				tokenAnalyzed++;
				if(execCounter > 1 && p->previous != NULL && p->previous->Pipe == TRUE){
					printf("Error: Process receive data from pipe and from file\n");
					exit(EXIT_FAILURE);
				}
			}
			// if |, > or >> is present
			if(tokenAnalyzed < tokenSize){
				tokenType = getType(tokens[tokenAnalyzed]);
				if(tokenType == PIPE) {
					// we don't need to use values just a flag
					p->Pipe = TRUE;
					tokenAnalyzed++;
				}
				else if(tokenType == OUTPUT_IN_FILE || tokenType == APPEND_IN_FILE) {
					tokenAnalyzed++;
					p->RedirectOutput = tokens[tokenAnalyzed];
					if(tokenType == OUTPUT_IN_FILE) {
						p->Overwrite = TRUE;
					}
					tokenAnalyzed++;
				}
			}
		}
		if(tokenAnalyzed < tokenSize)
		{
			p->next = malloc(sizeof(process));
			p->next->previous = p;
			p = p->next;
		}
	}
	if(strcmp(p->args[argCounter - 2],"&") == 0) {
		p->args[argCounter - 2] = NULL;
		*foreground = FALSE;
	}
	return firstProcess;
}
int tokenCounter(char *command) {
	int i = 0, word = 0;
	
	while(command[i] != '\0') {
		while(command[i] != '\0' && isspace(command[i])) i++;
		if(command[i] != '\0') word++;
		while(command[i] != '\0' && !isspace(command[i])) i++;
	}
	return word;
}

int readCommand(char ***tokens, char **cmd) {
	int token, i, resize = 0, isEndOfCommand = FALSE;
	char *tempCommand = malloc(sizeof(char) * DEFAULT_COMMAND_SIZE);
	char *command = NULL;	
	do {
		resize++;
		command = (char*)realloc(command, DEFAULT_COMMAND_SIZE * resize);
		if(resize == 1)
			command[0] = '\0';
		fgets(tempCommand, DEFAULT_COMMAND_SIZE, stdin);
		strcat(command, tempCommand);
		if(command[strlen(command)-1] == '\n') {
			command[strlen(command)-1] = '\0';
			isEndOfCommand = TRUE;
		}
	}while(!isEndOfCommand);
	free(tempCommand);
	token = tokenCounter(command);
	*cmd = malloc(sizeof(char) * (strlen(command) + 1));
	strcpy(*cmd, command);
	(*tokens) = (char**)malloc(sizeof(char*) * (token));
	if(token > 0) {
		(*tokens)[0]= strtok(command, " ");
	}
	for(i = 1; i < token; i++) {
		(*tokens)[i]= strtok(NULL, " ");
	}
	return token;
}
