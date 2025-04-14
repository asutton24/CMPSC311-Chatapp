
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <time.h>

#include <fcntl.h>

#include <pthread.h>







#define MAX_CLIENT 10 // Max number of clients that can connect to the server

#define PORT 8080 // Port number being accessed



int turnOnVerbose(void);

int turnOffVerbose(void);

int openLogFile(void);

int openLogFileTS(void);

int closeLogFile(void);

int writeToLog(char* buff);

int logSysTime(void);

int logUser(char* buff, char* user);

int logServer(char* buff);

int logError(char* buff);

int handleFlags(int argc, char** argv);





int client_socket[MAX_CLIENT] = {0}; // Array to store all the clients

pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER; // Protect client_socket being accessed at the same time

int client_count = 0; // Current number of clients





typedef struct

{

  int client_fd;

  int *client_count;

  int *client_sockets;

  pthread_mutex_t *client_lock;

} client_info_t;



// This function would be runned every time a new thread is created

void *handle_client(void* arg)

{

    int client_fd = *(int *)arg;

    char buffer[1024];

    char format_buffer[2048];

    ssize_t bytes_read;

    char name[100];



    logServer("Client connected...\n");



    // Receive the client's name once at the beginning

    bytes_read = recv(client_fd, name, sizeof(name) - 1, 0);

    if (bytes_read <= 0) {

        logServer("Failed to receive client name. Disconnecting client...\n");

        close(client_fd);

        return NULL;

    }

    name[bytes_read] = '\0';

    sprintf(format_buffer, "%s has joined the chat.\n", name);

    logServer(format_buffer);



    // Broadcast join message

    pthread_mutex_lock(&client_lock);

    for (int i = 0; i < MAX_CLIENT; i++) {

        if (client_socket[i] != 0 && client_socket[i] != client_fd) {

            send(client_socket[i], format_buffer, strlen(format_buffer), 0);

        }

    }

    pthread_mutex_unlock(&client_lock);



    // Main message loop

    while (1) {

        bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read <= 0) break;

        buffer[bytes_read] = '\0';



        sprintf(format_buffer, "%s: %s", name, buffer);

        logUser(buffer, name);



        pthread_mutex_lock(&client_lock);

        for (int i = 0; i < MAX_CLIENT; i++) {

            if (client_socket[i] != 0 && client_socket[i] != client_fd) {

                send(client_socket[i], format_buffer, strlen(format_buffer), 0);

            }

        }

        pthread_mutex_unlock(&client_lock);

    }



    // Handle client disconnect

    sprintf(format_buffer, "%s has left the chat.\n", name);

    logServer(format_buffer);

    pthread_mutex_lock(&client_lock);

    for (int i = 0; i < MAX_CLIENT; i++) {

        if (client_socket[i] == client_fd) {

            client_socket[i] = 0;

            break;

        }

    }

    client_count--;

    // Notify others about the disconnect

    for (int i = 0; i < MAX_CLIENT; i++) {

        if (client_socket[i] != 0) {

            send(client_socket[i], format_buffer, strlen(format_buffer), 0);

        }

    }

    pthread_mutex_unlock(&client_lock);

    close(client_fd);



    return NULL;

}



int main(int argc, char** argv) {

  const char* ip_address = "127.0.0.1";

  int socket_fd, new_socket;

  struct sockaddr_in server_address;

  socklen_t server_address_len = sizeof(server_address);

  pthread_t threads_id;

  int result;

  char format_buffer[2048];



  openLogFileTS();

  handleFlags(argc, argv);



  // First step create a socket

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {

    logError("Socket Error");

    exit(EXIT_FAILURE);

  }



  // Create the address struct

  memset(&server_address, 0, sizeof(server_address)); // Set's everything in the struct to 0

  server_address.sin_family = AF_INET;

  server_address.sin_addr.s_addr = inet_addr(ip_address);

  server_address.sin_port = htons(PORT);



  // Binding to IP address

  result = bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address));

  if (result < 0) {

    logError("Bind error");

    close(socket_fd);

    exit(EXIT_FAILURE);

  }



  // Listen for any connections

  result = listen(socket_fd, MAX_CLIENT);

  if (result == -1) {

    logError("Listen");

    close(socket_fd);

    exit(EXIT_FAILURE);

  }



  // Accepting the clients

  logServer("Server started, listening for connections...\n");



  // While loop to make new Threads

  while ((new_socket = accept(socket_fd, (struct sockaddr*)&server_address, &server_address_len)) >= 0)

  {

    pthread_mutex_lock(&client_lock);

    if (client_count < MAX_CLIENT)

    {

      client_socket[client_count++] = new_socket;

      printf("New Client connected, total clients: %d\n", client_count);



      // Create a thread to handle each client

      if (pthread_create(&threads_id, NULL, handle_client, (void *)&new_socket) != 0)

      {

        perror("Thread Creation Failed");

        logError("Thread Creation Failed");

        close(new_socket);

      }

      else

      {

        pthread_detach(threads_id);

      }

    }

    else

    {

      printf("Max client reached, rejection new connection\n");

      logError("Max client reached, rejection new connection\n");

      close(new_socket);

    }

    // Unlock the client to everyone before they connect

    pthread_mutex_unlock(&client_lock);

  }

  

  // if (client_fd < 0) {

  //   logError("Accept Error");

  //   close(socket_fd);

  //   exit(EXIT_FAILURE);

  // }

  //

  // logServer("Connection established...\n");

  //

  // while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0 ){

  //   buffer[bytes_read] = '\0';

  //   sprintf(format_buffer, "Received Data: %s", buffer);

  //   logServer(format_buffer);

  // }

  //

  // sprintf(format_buffer, "Socket bound to %s: %d\n", ip_address, PORT);

  // logServer(format_buffer);

  // close(socket_fd);

  // close(client_fd);



  close(socket_fd);

  closeLogFile();

  return 0;

}





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



//Writes error to log as well as perror

int logError(char* buff){

	int oldVerbose = verbosemode;

	verbosemode = 0;

	perror(buff);

	logServer(buff);

	writeToLog("\n");

	verbosemode = oldVerbose;

	return 0;

}





int handleFlags(int argc, char** argv){

	int argIndex = 1;

	while (argIndex < argc){

		if (!strcmp(argv[argIndex], "-v")){

			turnOnVerbose();

			logServer("Verbose mode active\n");

		}

		argIndex++;

	}

	return 0;

}
