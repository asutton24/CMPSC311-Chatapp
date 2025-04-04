#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int client_fd; // Global for all the threads to access
int track = 1; // Used to stop the thread once I type "exit"

// Get user inputs for server IP, port, and username
void client_inputs(char *server_ip, int *server_port, char *name) {
    printf("Enter the server IP address: ");
    scanf("%s", server_ip);

    printf("Enter the server port: ");
    scanf("%d", server_port);
    getchar(); // gets rid of newline

    printf("Enter your name: ");
    fgets(name, 100, stdin); 
    name[strlen(name) - 1] = '\0'; // removes newline
}

// Thread function for receiving data
void *read_thread_function(void *arg) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while (track) {
        bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("\nServer: %s\n", buffer);

        } 
        
    }

pthread_exit(0);
}


int main() {
    struct sockaddr_in server_addr;
    char SERVER_IP[20];
    char name[100];
    int SERVER_PORT;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent;

    pthread_t thread_read;

    // Get user input
    client_inputs(SERVER_IP, &SERVER_PORT, name);

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(client_fd);
        return -1;
    }

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_fd);
        return -1;
    }

    // Send username
    printf("Connected to the server as %s\n", name);
    send(client_fd, name, strlen(name) + 1, 0);

    // Start receiving thread
    pthread_create(&thread_read, NULL, read_thread_function, NULL);

    // Sending loop
    while (track) {
        sleep(1);
        printf("Enter message (or 'exit' to quit): ");
        fgets(buffer, sizeof(buffer), stdin);
        
        buffer[strlen(buffer) - 1] = '\0'; // remove newline

        if (strncmp(buffer, "exit", 4) == 0) {
            track = 0;
            break;
        }

        bytes_sent = send(client_fd, buffer, strlen(buffer) + 1, 0);
        if (bytes_sent == -1) {
            perror("Send failed");
            track = 0;

            break;
        }
    }

    pthread_cancel(thread_read);  // Terminates the thread.
    close(client_fd);
    printf("Disconnected from the server\n");

    return 0;
}