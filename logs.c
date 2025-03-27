#include "logs.h"

//Just tracks the file id of the log file
int logfd = 0;
//Verbose mode setting
int verbosemode = 0;

int turnOnVerbose(){
	verbosemode = 1;
	return 0;
}

int turnOffVerbose(){
	verbosemode = 0;
	return 0;
}

//Default server.log
int openLogFile(){
	if (logfd) return -1;
	logfd = open("server.log", O_WRONLY | O_APPEND | O_CREAT, 0774);
	return 0;
}

//Creates a timestamped .log file for a session
int openLogFileTS(){
	if (logfd) return -1;
	time_t timer;
	char buff[30];
	struct tm* info;
	timer = time(NULL);
	info = localtime(&timer);
	strftime(buff, 30, "%m-%d-%Y-%H:%M:%S.log", info);
	logfd = open(buff, O_WRONLY | O_APPEND | O_CREAT, 0774);
	return 0;
}

//Closes active log file
int closeLogFile(){
	if (!logfd) return -1;
	close(logfd);
	return 0;
}

//Logs to the log file and to console if needed. This is the base function for all other log functions
int writeToLog(char* buff){
	write(logfd, buff, strlen(buff));
	if (verbosemode) write(0, buff, strlen(buff));
	return 0;
}

//Just logs the system time
int logSysTime(){
	time_t timer;
	char buff[26];
	struct tm* info;
	timer = time(NULL);
	info = localtime(&timer);
	strftime(buff, 26, "%m-%d-%Y %H:%M:%S", info);
	writeToLog(buff);
}

//Log a timestamped message from a given user
int logUser(char* buff, char* user){
	logSysTime();
	writeToLog(" ");
	writeToLog(user);
	writeToLog(": ");
	writeToLog(buff);
	return 0;
}

//Logs a message from user "Server"
int logServer(char* buff){
	logUser(buff, "Server");
	return 0;
}
