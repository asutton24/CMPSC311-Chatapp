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

int main() {
  const char* ip_address = "127.0.0.1";
  int port = 8080;
  int socket_fd;
  int client_fd;
  char buffer[1024];
  char format_buffer[2048];
  struct sockaddr_in server_address;
  struct sockaddr_storage client_address;
  socklen_t client_address_len = sizeof(client_address);
  ssize_t bytes_read;
  int result;

  openLogFileTS();
  // First step create a socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    logError("Socket Error");
    exit(EXIT_FAILURE);
  }

  // Create the adress struct
  memset(&server_address, 0, sizeof(server_address)); // Set's everything in the struct to 0
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(ip_address);
  server_address.sin_port = htons(port);

  // Binding to IP adress
  result = bind(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address));
  
  result = listen(socket_fd, 5);

  if (result < 0) {
    logError("Bind error");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  logServer("Wait for connection...\n");
  client_fd = accept(socket_fd, (struct sockaddr*)&client_address, sizeof(client_address_len));
  
  if (client_fd < 0) {
    logError("Accept Error");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  logServer("Connection established...\n");

  while (bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0) > 0 ){
    buffer[bytes_read] = '\0';
    sprintf(format_buffer, "Received Data: %s", buffer);
    logServer(format_buffer);
  }

  sprintf(format_buffer, "Socket bound to %s: %d\n", ip_address, port);
  logServer(format_buffer);
  close(socket_fd);
  close(client_fd);

  closeLogFile();
  return 0;
}


