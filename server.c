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
#include <pthread.h>

#define MAX_CLIENT 10
#define PORT 8080

int client_socket[MAX_CLIENT] = {0};
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

typedef struct
{
  int client_fd;
  int *client_count;
  int *client_sockets;
  pthread_mutex_t *client_lock;
} client_info_t;

void *handle_client(void* arg)
{
  int client_fd = *(int *)arg;
  char buffer[1024];
  char format_buffer[2048];
  ssize_t bytes_read;

  logServer("Client connected...\n");

  while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0 ){
    buffer[bytes_read] = '\0';
    sprintf(format_buffer, "Received Data: %s", buffer);
    logServer(format_buffer);
  }

  logServer("Client disconnected...\n");

  // Remove client from the list
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < MAX_CLIENT; i++)
  {
    if (client_socket[i] == client_fd)
    {
      client_socket[i] = 0;
      break;
    }
  }

  client_count--;
  pthread_mutex_unlock(&client_lock);
  close(client_fd);

  return NULL;

}

int main() {
  const char* ip_address = "127.0.0.1";
  int socket_fd, new_socket;
  struct sockaddr_in server_address;
  socklen_t server_address_len = sizeof(server_address);
  pthread_t threads_id;
  int result;

  // char buffer[1024];
  // char format_buffer[2048];
  // ssize_t bytes_read;

  openLogFileTS();
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


