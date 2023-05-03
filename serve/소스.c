#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void error(const char* message) {
    perror(message);
    exit(1);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength;
    char buffer[BUFFER_SIZE];

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        error("Error creating server socket");
    }

    // Set server address properties
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8888);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to a specific address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("Error binding server socket");
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        error("Error listening for connections");
    }

    printf("Chat server started. Listening on port 8888...\n");

    // Accept client connections and start chat
    while (1) {
        // Accept a client connection
        clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            error("Error accepting client connection");
        }

        printf("Client connected. Client address: %s\n", inet_ntoa(clientAddress.sin_addr));

        // Start chat
        while (1) {
            // Receive message from client
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0) {
                error("Error receiving message");
            }

            printf("Client: %s", buffer);

            // Check if client wants to exit
            if (strcmp(buffer, "exit\n") == 0) {
                break;
            }

            // Read message from server
            printf("Server: ");
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);

            // Send message to client
            if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
                error("Error sending message");
            }

            // Check if server wants to exit
            if (strcmp(buffer, "exit\n") == 0) {
                break;
            }
        }

        // Close client socket
        close(clientSocket);
        printf("Client disconnected.\n");
    }

    // Close server socket
    close(serverSocket);

    return 0;
}

