#include "builtin.h"
#include "jobControl.h"


void cd(int argSize, char **args) {
	if(argSize == 2) {
		char *currentDir;
		char *pos, *newDir;
		currentDir = getcwd(NULL, 0); 
		if(strcmp(args[1], "..") == 0) {
			
			newDir = malloc(sizeof(char) * (strlen(currentDir)));
			strcpy(newDir, currentDir);
			pos = strrchr(newDir, '/');
			*pos = '\0';
		}
		else {
			newDir = malloc(sizeof(char) * (strlen(currentDir) + strlen(args[1]) + 2)); // '\0' e / do novo diretório
			strcpy(newDir, currentDir);
			strcat(newDir, "/");
			strcat(newDir, args[1]);
		}
		chdir(newDir);		
		free(newDir);		
		free(currentDir);
		//TODO exibir mensagem de erro caso não seja possível alterar o diretório atual	
	} else
		printf("Error: Invalid number of operands");
}

void pwd() {
	char *currentDir = getcwd(NULL, 0);
	printf("%s\n", currentDir);
}

void jobsExhibition(){
	job *j;
	int i = 0;
	j = jobs;
	while(j != NULL && j->firstProcess != NULL) {
		if(jobIsCompleted(j) == 1)
			printf("[%d]\tCompleted \t%s\n", i, j->commandLine);
		else if(jobIsStopped(j) == 1)
			printf("[%d]\tStopped \t%s\n", i, j->commandLine);
		else
			printf("[%d]\tRunning \t%s\n", i, j->commandLine);
		j = j->next;
		i++;
	}
}


int chooseBuiltin(char *commandName) {

	if(strcmp(commandName, "exit") == 0) return 0;
	if(strcmp(commandName, "cd") == 0) return 1;
	if(strcmp(commandName, "pwd") == 0) return 2;
	if(strcmp(commandName, "jobs") == 0) return 3;
	if(strcmp(commandName, "fg") == 0) return 4;
	if(strcmp(commandName, "bg") == 0) return 5;
	return -1;
}
// TODO change all error messages

void fg(int argSize, char **args) {
	job *j = jobs;
	if(argSize != 2) 
		printf("Error: Invalid fg parameters\n");
	else
	{
		int indexJob = atoi(args[0]);
		if(indexJob >= 0) {
			int i = 0;
			for(i = 0; i < indexJob && j != NULL; i++)
				j = j->next;
			if(i == indexJob) {			
				process *p = j->firstProcess;
				while(p != NULL) {
					p->stopped = FALSE;
					p = p->next;
				}		
				putJobInForeground(j, 1);
			}
		}
		else
			printf("Error: Negative index for job!\n");	
	}
}

void bg(int argSize, char **args) {
	job *j = jobs;
	if(argSize != 2) 
		printf("Error: Invalid fg parameters\n");
	else
	{
		int indexJob = atoi(args[0]);
		if(indexJob >= 0) {
			int i = 0;
			for(i = 0; i < indexJob && j != NULL; i++)
				j = j->next;
			if(i == indexJob) {
				process *p = j->firstProcess;
				while(p != NULL) {
					p->stopped = FALSE;
					p = p->next;
				}
				putJobInBackground(j, 1);
			}
		}
		else
			printf("Error: Negative index for job!\n");	
	}
}

int executeBuiltIn(int argSize,char *commandName, char **args) {
	int command = chooseBuiltin(commandName);
	
	switch(command) {
		case 0:
			clearAllJobs();
			exit(EXIT_SUCCESS);
			return 1;
			break;
		case 1:
			cd(argSize, args);
			return 1;
			break;
		case 2:
			pwd();
			return 1;
			break;
		case 3:
			jobsExhibition();
			return 1;
			break;
		case 4:
			fg(argSize, args);
			return 1;
			break;
		case 5:
			bg(argSize, args);
			return 1;
			break;
	}
	return 0;
}

