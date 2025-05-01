#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int new_socket;
char buffer[BUFFER_SIZE];

void* send_messages(void* arg) {
    while (1) {
        printf("Enter message to send to client: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character from the input

        // If the server types "bye", close the connection
        if (strcmp(buffer, "bye") == 0) {
            send(new_socket, buffer, strlen(buffer), 0);
            printf("You terminated the connection.\n");
            close(new_socket);
            exit(0);
        }

        // Send message to client
        send(new_socket, buffer, strlen(buffer), 0);
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int port;
    char ip_address[16]; // Buffer for IPv4 address (e.g., "192.168.1.100")

    // Prompt for IP address
    printf("Enter IP address to bind (e.g., 127.0.0.1): ");
    fgets(ip_address, sizeof(ip_address), stdin);
    ip_address[strcspn(ip_address, "\n")] = 0; // Remove newline

    // Prompt for port number
    printf("Enter port number (1024-65535): ");
    scanf("%d", &port);
    getchar(); // Consume newline character left by scanf

    // Validate port number
    if (port < 1024 || port > 65535) {
        printf("Invalid port number. Port must be between 1024 and 65535.\n");
        return 1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure address structure
    address.sin_family = AF_INET;
    // Convert IP address from string to binary
    if (inet_pton(AF_INET, ip_address, &address.sin_addr) <= 0) {
        printf("Invalid IP address format or address not supported.\n");
        close(server_fd);
        return 1;
    }
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on %s:%d\n", ip_address, port);

    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    // Create a thread to handle sending messages to the client
    pthread_t send_thread;
    pthread_create(&send_thread, NULL, send_messages, NULL);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }
        printf("Client: %s\n", buffer);

        // If the client sends "bye", terminate the connection
        if (strcmp(buffer, "bye") == 0) {
            printf("Client terminated the connection.\n");
            close(new_socket);
            break;
        }
    }

    // Wait for the send thread to finish
    pthread_join(send_thread, NULL);

    // Close socket
    close(server_fd);

    return 0;
}