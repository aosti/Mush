#include "mush.h"

void launchProcess(process *p, pid_t pgid, int foreground) {
        if(shellIsInteractive) {
		p->programToExecute = findPath(&(p->programToExecute));			// Find the path to an executable file
		if(p->programToExecute == NULL) {
			printf("Error: can't find executable!\n");
			exit(EXIT_FAILURE);
		}
       		pid_t pid = getpid();
	        if (pgid == 0) 
			pgid = pid;
                setpgid(pid, pgid);
	        if (foreground)
                        tcsetpgrp(shellTerminal, pgid);
		
		execv(p->programToExecute, p->args);
        }
	exit(EXIT_FAILURE);
}

char *findPath(char **program) {
	char *PATH, *localPath;
	char **lookAt;
	char *completePath;
	int i, directory = 1;
	struct stat buf;		// This variable holds information about the program user is trying to execute

	if((*program)[0] == '.' && (*program)[1] == '/') {	// user is executing a program in the current folder	
		return *program;
	}
	PATH = getenv("PATH");
	localPath = malloc((strlen(PATH) + 1) * sizeof(char));
	strcpy(localPath, PATH);		// According to libc manual the return value of getenv must not be modified	
	if(strlen(localPath) == 0) {
		printf("Error: PATH variable is empty!\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; localPath[i] != '\0'; i++)		
		if(localPath[i] == ':')
			directory++;
	lookAt = malloc(sizeof(char*) * directory);
	lookAt[0] = strtok(localPath, ":");
	for(i = 1; i < directory; i++)
		lookAt[i] = strtok(NULL, ":");
	// Try to find the executable looking at the folders that are in path
	for(i = 0; i < directory ; i++) {
		completePath = malloc(sizeof(char) * (strlen(lookAt[i]) + strlen(*program) + 2)); // +2 to additional / and \0
		strcpy(completePath, lookAt[i]);
		strcat(completePath, "/");
		strcat(completePath, *program);
		if(stat(completePath, &buf) == 0){ 	// Found complete path to executable
			free(lookAt);
			free(localPath);
			return completePath;
		}
		else {
			free(completePath);
		}
	}

	return NULL;	
}

void executeCommand(char **tokens, int tokenSize, job **lastJob, char *command) {
	int pid, foreground;
	process *p;
	p = parseCommand(tokenSize, tokens, &foreground);	
		
	//printf("%s", tokens[0]);

	if(executeBuiltIn(tokenSize, tokens[0], tokens) == 0) {
		int pipefd[2][2];	// [0] used by process who wants to send output to other process
					// [1] used by process who wants to receive input from other process
		
		int fd[2];	// [0] used by process who wants to receive input from file
				// [1] used by process who wants to output to file
		(*lastJob)->firstProcess = p;
		(*lastJob)->commandLine = command;
		(*lastJob)->next = initJob();
		while( p != NULL) {	
			//This tests if last program used pipe	
			if(p->previous != NULL && p->previous->Pipe)
			{
				pipefd[1][PIPE_READ] = dup(pipefd[0][PIPE_READ]);
				close(pipefd[0][PIPE_WRITE]);
				close(pipefd[0][PIPE_READ]);
			}	
			else if(p->previous != NULL && p->previous->Pipe == FALSE){
				close(pipefd[0][PIPE_WRITE]);
				close(pipefd[0][PIPE_READ]);
			}
			//If current program wants to create a pipe
			if(p->Pipe == TRUE)
				pipe(pipefd[0]);
			
			//If current program reads from file
			if(p->RedirectInput != NULL) {
				//printf("%s\n", p->RedirectInput[i]);
				fd[0] = open(p->RedirectInput, O_RDONLY);
				if(fd[0] == -1) {
					printf("Error: File not found\n");
					exit(EXIT_FAILURE);
				}	
			}
			// If current program writes on file
			if(p->RedirectOutput != NULL) {
				//printf("%s\n", p->RedirectOutput);
				if(p->Overwrite == TRUE)
					fd[1] = open(p->RedirectOutput, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);	// If user used >
				else {
					fd[1] = open(p->RedirectOutput, O_CREAT|O_WRONLY|O_APPEND, S_IRWXU);	// If user used >>
				}
				if(fd[1] == -1) {
					printf("Error: File not found\n");
					exit(EXIT_FAILURE);
				}
			}
			pid = fork();
			if(pid < 0)
			{
				 printf("%s", strerror(errno));
			}	
			else if(pid > 0)
			{
				// This is the parent process
				p->pid = pid;
				if(shellIsInteractive) {
					if(!(*lastJob)->pgid){
						(*lastJob)->pgid = pid;
					}
					setpgid(pid, (*lastJob)->pgid);
				}
				p = p->next;			
			}
			else if(pid == 0) {
				sigset_t new_mask;				
				// Child will receive all the signals
				sigemptyset (&new_mask);
  				sigprocmask (SIG_SETMASK, &new_mask, NULL);
				//sigaction(SIGTSTP, &oact, NULL);		
				//Child's code
				if(p->Pipe == TRUE) {
					close(1);	// Close standart output
					dup(pipefd[0][PIPE_WRITE]);
					close(pipefd[0][PIPE_WRITE]);
					close(pipefd[0][PIPE_READ]);
				}
				//VER O QUE VAI ACONTECER PARA O REDIRECIONAMENTO PARA ARQUIVO
				if(p->previous != NULL && p->previous->Pipe == TRUE) 
				{
					close(0);	// Close standart input
					dup(pipefd[1][PIPE_READ]);
					close(pipefd[1][PIPE_READ]);
				}
				//Ver o caso em que o processo anterior envia dados para o atual e o atual tem redirecionamento de entrada para arquivo
				if(p->RedirectInput != NULL) {
					close(0);
					dup(fd[0]);
					close(fd[0]);
				}
				if(p->RedirectOutput != NULL) {
					close(1);
					dup(fd[1]);
					close(fd[1]);
				}
				launchProcess(p, (*lastJob)->pgid, foreground);	
				//colocar mensagem de erro se não conseguir executar
			}
		}
		if (foreground) {
			//printf("Waiting...\n");
			putJobInForeground (*lastJob, 0);
		}
		else
			putJobInBackground (*lastJob, 0);
		(*lastJob) = (*lastJob)->next;
	}
	else {		// It was a builtin command
		clearProcess2(p);		
	}
	
}

int main(int argc, char **argv) {
	char **tokenized_command, tokenSize, **command = NULL;
	job *lastJob;
	int i = 0;

	initShell();
	lastJob = jobs;
	printf("Version 1.0 Mush\n");
	while(TRUE) {
		printf("$");
		command = realloc(command, sizeof(char*) * (i + 1));
		tokenSize = readCommand(&tokenized_command, &command[i]);
		if(tokenSize != 0) {
			executeCommand(tokenized_command, tokenSize, &lastJob, command[i]);
		}
		doJobNotification();
	}
	return 0;
}
