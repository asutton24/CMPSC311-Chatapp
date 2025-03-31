#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
   
//User inputs for server Ip address, port, name
void client_inputs(char *server_ip, int *server_port, char *name) {
    
    printf("Enter the server IP address: ");
    scanf("%s", server_ip);
    
    printf("Enter the server port: ");
    scanf("%d", server_port);
    
    printf("Enter your name: ");
    scanf("%s", name);
}

#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char server_ip[20];
    char name[100];
    int server_port;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent;

    // Call function to get user inputs
    client_inputs(server_ip, &server_port, name);

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(client_fd);
        return -1;
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_fd);
        return -1;
    }

    printf("Connected to the server as %s\n", name);

    // Read and send data to the server
    while (1) {
        printf("Enter message (or 'exit' to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Check for exit condition
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        // Send data to the server
        bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
        if (bytes_sent == -1) {
            perror("Send failed");
            break;
        }
    }

    // Close the socket
    close(client_fd);
    printf("Disconnected from the server\n");
    return 0;
}