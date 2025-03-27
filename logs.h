#ifndef LOGS_H
#define LOGS_H

#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int turnOnVerbose(void);
int turnOffVerbose(void);
int openLogFile(void);
int openLogFileTS(void);
int closeLogFile(void);
int writeToLog(char* buff);
int logSysTime(void);
int logUser(char* buff, char* user);
int logServer(char* buff);

#endif
