#include "jobControl.h"

job *findJob(pid_t pgid, job *j) {
        while(j != NULL && j->pgid != pgid)
                j = j->next;
        return j;
}

void stopJob(int signum){
//	sigset_t mask, old;
//	sigfillset(&mask);
//	setprocmask(
}

int jobIsStopped(job *j) {
        process *p;
	if(j->firstProcess != NULL)
		for(p = j->firstProcess; p != NULL; p = p->next) {
		        if(!p->completed && !p->stopped)
		                return 0;
		}
	else
		return 0;		// No process in this job
        return 1;       //Job is stopped or completed
}
        
int jobIsCompleted(job *j) {
        process *p;

	if(j->firstProcess != NULL)
		for(p = j->firstProcess; p != NULL; p = p->next) {
		        if(!p->completed)
		                return 0;
		}
	else
		return 0;		// No process in this job
//	printf("All Completed!\n");
        return 1;       //All process in the job have completed
}

void formatJobInfo(job *j, int state) {
	//
}

void waitForJob(job *j) {
	int status;
	pid_t pid;

	do {
		pid = waitpid(WAIT_ANY, &status, WUNTRACED);
	}while(!markProcessStatus(pid, status) && !jobIsStopped(j) && !jobIsCompleted(j));
}
void putJobInForeground(job *j, int cont) {
	//Put the job in the foreground
	tcsetpgrp(shellTerminal, j->pgid);

	if(cont) {
		//Send the job a continue signal, if necessary
		if(kill(-j->pgid, SIGCONT) < 0)
			printf("Error: SIGCONT\n");
	}

	waitForJob(j);

	//Put the shell in the foreground
	tcsetpgrp(shellTerminal, shellPgid);
	
	//Restore the shell's terminal modes
	tcsetattr (shellTerminal, TCSADRAIN, &shellTmodes);	
}

job *initJob() {
	job *j = malloc(sizeof(job));
	j->next = NULL;
	j->commandLine = NULL;
	j->firstProcess = NULL;
	j->pgid = 0;	
	j->notified = FALSE;
	return j;
}

void initShell() {
        shellTerminal = STDIN_FILENO;
        shellIsInteractive = isatty(shellTerminal);
	jobs = initJob();
     
        jobs->commandLine = NULL;
	if(shellIsInteractive) {
		struct sigaction act, oact;
		sigset_t new_mask, old_mask;		
		if(memset(&act, 0, sizeof(struct sigaction)) == NULL)
			printf("Error in signal handler creation!\n");
		              
		/* Loop until we are in the foreground. */
                while( tcgetpgrp(shellTerminal) != (shellPgid = getpgrp()))
                        kill (-shellPgid, SIGTTIN);     // Send signal to stop all process in the shell group
                
		// Put ourselves in our own process group
                shellPgid = getpid();
                if(setpgid(shellPgid, shellPgid) < 0) {
                        printf("Error: Couldn't put the shell in its own process group");
                        exit(EXIT_FAILURE);                     
                }

                // Grab control of the terminal
                tcsetpgrp(shellTerminal, shellPgid);
        
                // Save default terminal attributes for shell.
                tcgetattr(shellTerminal, &shellTmodes);
	
		// Ignore interactive and job-control signals.
		act.sa_handler = stopJob;
		sigaction(SIGTSTP, &act, &oact); /* SIGTSTP is ^Z */

		sigfillset (&new_mask);
  		sigdelset (&new_mask, SIGTSTP);
  		sigprocmask (SIG_BLOCK, &new_mask, &old_mask);
        }       
}

void putJobInBackground(job *j, int cont) {
	//Send the job a continue signal, if necessary
	if(cont)
		if(kill(-j->pgid, SIGCONT) < 0) {
			printf("kill (SIGCONT)\n");
		}
}

int markProcessStatus(pid_t pid, int status) {

	job *j;
	process *p;		
	
	if(pid > 0) {
		for(j = jobs; j != NULL ; j = j->next)
			for(p = j->firstProcess; p != NULL; p = p->next)
				if(p->pid == pid) {
					p->status = status;
					if(WIFSTOPPED(status))
						p->stopped = TRUE;
					else {
						p->completed = TRUE;
						if(WIFSIGNALED(status))
							printf("%d: Terminated by signal %d\n", (int)pid, WTERMSIG(p->status)); // Improve this
					}
					return 0;
				}
		printf("No child process. %d\n", pid);
	}
	else if(pid == 0 || errno == ECHILD) {
		return -1;		
	}
	else {
		printf("error: waitpit\n");
		return -1;

	}
	return -1;
}	

void updateStatus(){
	int status;
	pid_t pid;
	
	do 
		pid = waitpid(WAIT_ANY, &status, WUNTRACED|WNOHANG);
	while(!markProcessStatus(pid, status));
}

void doJobNotification() {
	job *j, *jlast, *jnext;
	updateStatus();

	jlast = NULL;
	if(jobs != NULL)
		for(j = jobs; j != NULL; j = jnext) {
			jnext = j->next;
			if(jobIsCompleted(j)) {
				//printf("Job completed\n");
				if(jlast)
					jlast->next = jnext;				
				else 
					jobs = j->next;
				clearProcess1(j);
				free(j);
			}
			else if(jobIsStopped(j) && !j->notified) {
				//printf("Job stopped\n");
				j->notified = TRUE;
				jlast = j;
			}
			else jlast = j;
		}
	else
		printf("fuck\n");
}

void clearAllJobs() {
	job *j = jobs;
	job *previous;
	
	while(j != NULL) {
		previous = j;
		clearProcess1(j);
		j = j->next;
		free(previous);
	}
}
void clearProcess1(job *j) {
	process *p = j->firstProcess;
	process *previous;
	while(p != NULL) {
		previous = p;
		p = p->next;
		free(previous);
	}
}

void clearProcess2(process *p) {
	process *previous;
	while(p != NULL) {
		previous = p;
		p = p->next;
		free(previous);
	}
}
