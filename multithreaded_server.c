#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void print_error_and_exit(const char *message) {
    perror(message);
    exit(1);
}

void *handle_client(void *client_sock) {
    int sock = *(int*)client_sock;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive data from the client
    bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        print_error_and_exit("Error receiving data from client");
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received message

    printf("Received message: %s\n", buffer);

    // Send the received message back to the client
    if (send(sock, buffer, bytes_received, 0) < 0) {
        print_error_and_exit("Error sending data to client");
    }

    printf("Echoed message to client\n");

    // Close the client socket
    close(sock);
    free(client_sock);
    return NULL;
}

int main() {
    char server_ip[16];
    int server_port;
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    // Prompt user for server details
    printf("Enter the server IP address (e.g., 127.0.0.1): ");
    scanf("%s", server_ip);
    printf("Enter the server port number: ");
    scanf("%d", &server_port);

    // Create a socket for the server
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print_error_and_exit("Error creating socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Bind the socket to the specified address and port
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        print_error_and_exit("Bind failed");
    }

    // Start listening for incoming connections
    if (listen(server_sock, 10) < 0) {
        print_error_and_exit("Listen failed");
    }

    printf("Server listening on %s:%d...\n", server_ip, server_port);

    // Accept incoming client connections in an infinite loop
    while (1) {
        client_sock = (int*)malloc(sizeof(int));
        if ((*client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
            print_error_and_exit("Accept failed");
        }

        printf("Client connected\n");

        // Create a new thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, (void*)client_sock) != 0) {
            print_error_and_exit("Thread creation failed");
        }

        // Detach the thread to allow it to clean up itself when done
        pthread_detach(tid);
    }

    close(server_sock);
    return 0;
}
