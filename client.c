#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Server's IP address
#define SERVER_PORT 8080 // Server's port
#define BUFFER_SIZE 1024


int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent;

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(client_fd);
        return -1;
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
    -1) {
        perror("Connection failed");
        close(client_fd);
        return -1;
    }

    printf("Connected to the server\n");
    send(client_fd, "\x02NAMEandrew", 11, 0);
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
