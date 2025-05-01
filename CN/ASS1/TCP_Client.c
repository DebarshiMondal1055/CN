#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int sock;
char buffer[BUFFER_SIZE];

void* receive_messages(void* arg) {
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';  // Null-terminate the received message
            printf("Server: %s\n", buffer);
            if (strcmp(buffer, "bye") == 0) {
                printf("Server terminated the connection.\n");
                close(sock);
                exit(0);
            }
        }
    }
}

int main() {
    struct sockaddr_in serv_addr;
    int port;
    char ip_address[16]; // Buffer for IPv4 address (e.g., "192.168.1.100")

    // Prompt for IP address
    printf("Enter server IP address (e.g., 127.0.0.1): ");
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
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return -1;
    }

    printf("Connected to server at %s:%d!\n", ip_address, port);

    // Create a thread to handle receiving messages from the server
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character from the input

        // If the client types "bye", close the connection
        if (strcmp(buffer, "bye") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            printf("You terminated the connection.\n");
            close(sock);
            break;
        }

        // Send message to server
        send(sock, buffer, strlen(buffer), 0);
    }

    // Wait for the receive thread to finish
    pthread_join(recv_thread, NULL);

    return 0;
}