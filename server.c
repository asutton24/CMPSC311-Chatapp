// Roberto Ramirez
// Andrew Sutton
// Daniel Vasquez
// Jeremy Veloso

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include "logs.h"
#include "flags.h"
#include <pthread.h>
#include <signal.h>

#define MAX_CLIENT 10 // Max number of clients that can connect to the server
#define PORT 8080 // Port number being accessed
volatile sig_atomic_t shutting_down = 0;

typedef struct
{
  int client_fd;
  char name[32];
} user_t;

int client_socket[MAX_CLIENT] = {0}; // Array to store all the clients
char names_list[MAX_CLIENT][32] = {0};
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER; // Protect client_socket being accessed at the same time
int client_count = 0; // Current number of clients

typedef struct
{
  int client_fd;
  int *client_count;
  int *client_sockets;
  pthread_mutex_t *client_lock;
} client_info_t;

void dummy_function(){return;}

int messageFromSender(user_t* user, char* buffer){
  int bufflen = strlen(buffer) + strlen(user->name) + 6 + 5 * (strlen(user->name) == 0);
  int orglen = strlen(buffer);
  if (bufflen > 1023) return -1;
  for (int i = orglen - 1; i >= 0; i--){
    buffer[i + bufflen - orglen] = buffer[i];
  }
  strcpy(buffer, "\x02USER");
  if (strlen(user->name) == 0){
     strcpy(buffer + 5, "guest");
     buffer[10] = 2;
     logUser(buffer + 11, "guest");
  } else {
    strcpy(buffer + 5, user->name);
    buffer[5 + strlen(user->name)] = 2;
    logUser(6 + strlen(user->name) + buffer, user->name);
  }
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < MAX_CLIENT; i++){
    if (client_socket[i] != user->client_fd && client_socket[i] != 0) send(client_socket[i], buffer, bufflen, 0);
  }
  pthread_mutex_unlock(&client_lock);
  return 0;
}

int decodeMessage(user_t* user, char* buffer){
  char return_buffer[400];
  int client_count = -1;
  int buffer_index = 7;
  if (buffer[0] == 2){
    if (strncmp(buffer + 1, "NAME", 4) == 0){
      if (strlen(buffer + 5) > 31){
        return -1;
      }
      strcpy(user->name, buffer + 5);
      for (int i = 0; i < 32; i++){
        if (user->name[i] == '\n' || user->name[i] == '\r'){
          user->name[i] = 0;
          break;
	}
      }
    } else if (strncmp(buffer + 1, "LIST", 4) == 0){
      for (int i = 0; i < MAX_CLIENT; i++){
        if (names_list[i][0] != 0) client_count++;
      }
      strcpy("\x02LIST0\x02", return_buffer);
      return_buffer[5] += client_count;
      for (int i = 0; i < MAX_CLIENT; i++){
        if (names_list[i][0] != 0){
          strcpy(names_list[i], return_buffer + buffer_index);
          buffer_index += strlen(names_list[i]);
          return_buffer[buffer_index] = 2;
          buffer_index++;
        }
      }
      send(user->client_fd, return_buffer, strlen(return_buffer), 0);
    }
    return 0;
  }
  return messageFromSender(user, buffer);
}

void triggerDummyConnection()
{
  int dummy_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (dummy_fd < 0)
  {
    logError("Dummy Socket creation failed");
    return;
  }

  struct sockaddr_in dummy_addr;
  memset(&dummy_addr, 0, sizeof(dummy_addr));
  dummy_addr.sin_family = AF_INET;
  dummy_addr.sin_port = htons(PORT);
  dummy_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  connect(dummy_fd, (struct sockaddr *)&dummy_addr, sizeof(dummy_addr));
  close(dummy_fd);

}

void shutdownServer(int server_socket_fd)
{
  shutting_down = 1;
  logServer("Shutting down server... \n");

  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < MAX_CLIENT; i++)
  {
    // Loop through the connected client informing them that the server is shutting down
    // Also disconnect the clients from the server
    if (client_socket[i] != 0)
    {
      send(client_socket[i], "Server Shutting down... \n", sizeof("Server Shutting down... \n"), 0);
      shutdown(client_socket[i], SHUT_RDWR);
      close(client_socket[i]);
      client_socket[i] = 0;
    }
  }

  pthread_mutex_unlock(&client_lock);

  shutdown(server_socket_fd, SHUT_RDWR);
  close(server_socket_fd);
  closeLogFile();

  printf("Server Shutdown successfully\n");
  logServer("Server Shutdown successfully\n");
  triggerDummyConnection();
}

void *commandListener(void* arg)
{
  int server_socket_fd;
  server_socket_fd = *(int *)arg;
  char terminal[64];

  while (!shutting_down && fgets(terminal, sizeof(terminal), stdin))
  {
    if (strncmp(terminal, "shutdown", strlen("shutdown")) == 0)
    {
      logServer("Shutdown command received form the terminal.\n");
      shutdownServer(server_socket_fd);
    }
    else
    {
      logError("Unknown Command\n");
      printf("Unknown Command: %s\n", terminal);
    }
  }

  return NULL;
}

// This function would be run every time a new thread is created
void *handle_client(void* arg)
{
  user_t user;
  user.client_fd = *(int *)arg;
  char buffer[1024];
  char format_buffer[2048];
  ssize_t bytes_read;

  logServer("Client connected...\n");


  // Receive message from the client until they disconnect
  while (!shutting_down && (bytes_read = recv(user.client_fd, buffer, sizeof(buffer) - 1, 0)) > 0 ){
    buffer[bytes_read] = '\0';
    sprintf(format_buffer, "Received Data: %s", buffer);
    logServer(format_buffer);
    decodeMessage(&user, buffer);

    // Check who sent the message, and then send the message to everyone expect who sent the message.
    //pthread_mutex_lock(&client_lock);
    //for (int i = 0; i < MAX_CLIENT; i++) {
    //  if (client_socket[i] != 0 && client_socket[i] != user.client_fd) {
    //    send(client_socket[i], buffer, bytes_read, 0); // Send to all clients
    //   }
    //}

    //pthread_mutex_unlock(&client_lock);
    memset(buffer, 0, sizeof(buffer));
  }

  logServer("Client disconnected...\n");

  // Remove client from the list
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < MAX_CLIENT; i++)
  {
    if (client_socket[i] == user.client_fd)
    {
      client_socket[i] = 0;
      memset(names_list[i], 0, 32);
      break;
    }
  }

  client_count--;
  pthread_mutex_unlock(&client_lock);
  close(user.client_fd);

  return NULL;

}

int main(int argc, char** argv) {
  const char* ip_address = "192.168.40.229";
  int socket_fd, new_socket;
  struct sockaddr_in server_address;
  socklen_t server_address_len = sizeof(server_address);
  pthread_t threads_id;
  int result;
  char format_buffer[2048];
  signal(SIGINT, dummy_function);
  openLogFileTS();
  handleFlags(argc, argv);
  //Zero out the name list
  for (int i = 0; i < MAX_CLIENT; i++){
    for (int j = 0; j < 32; j++){
      names_list[i][j] = 0;
    }
  }
  // First step create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    logError("Socket Error");
    exit(EXIT_FAILURE);
  }

  // Start the command listener to await any shutdown commands
  pthread_t command_thread;
  pthread_create(&command_thread, NULL, commandListener, &socket_fd);

  // Create the address struct
  memset(&server_address, 0, sizeof(server_address)); // Set's everything in the struct to 0
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
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

  pthread_join(command_thread, NULL);
  return 0;
}


