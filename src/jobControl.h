#ifndef JOB_CONTROL_H_
#define JOB_CONTROL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define LAUNCHED	1
#define TRUE		1
#define FALSE		0	
// This code is entirely based on libc manual pages about job control

// See if it's necessary to have this as global variables
pid_t shellPgid;
int shellTerminal;
int shellIsInteractive;
struct termios shellTmodes;


typedef struct process {
        struct process *next;
        struct process *previous;
        char *programToExecute;
        char **args;
        int pid;
        int completed;
        int stopped;
        int status;
        int Pipe;
        char *RedirectInput;
        char *RedirectOutput;
        int Overwrite;
} process;

typedef struct job {
	struct job *next;
	char *commandLine;
	process *firstProcess;
	pid_t pgid;	
	int notified;	// true if user told to stop a job
} job;

void stopJob(int signum);

job *jobs;
job *findJob(pid_t pgid, job *j);
int jobIsStopped(job *j);
int jobIsCompleted(job *j);
void formatJobInfo(job *j, int state);
void initShell();
job *initJob();
void waitForJob(job *j);
void doJobNotification();
void putJobInForeground(job *j, int cont);
void putJobInBackground(job *j, int cont);
void clearAllJobs();
void clearProcess1(job *j);
void clearProcess2(process *p);
int markProcessStatus(pid_t pid, int status);
#endif
