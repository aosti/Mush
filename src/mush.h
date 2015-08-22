#ifndef MUSH_H_
#define MUSH_H_

#include "parser.h"
#include "builtin.h"
#include "jobControl.h"

void launchProcess(process *p, pid_t pgid, int foreground);
char *findPath(char **program);
void executeCommand(char **tokens, int tokenSize, job **lastJob, char *command);

#endif
