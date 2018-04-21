#include "debug.h"
#include "tcp.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct sockaddr_in gAddress;
int gSocket;
void *(*gHandler)(void *);

void InitServer(int port, void *(*handler)(void *)) 
{
    // Create socket
    gSocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);

    if (gSocket == -1) {
        printf("Could not create socket\n");
        return;
    } else {
        printf("Socket created with port %d\n", port);
    }

    // Prepare the sockaddr_in structure
    gAddress.sin_family = AF_INET;
    gAddress.sin_addr.s_addr = INADDR_ANY;
    gAddress.sin_port = htons(port);

    // Bind
    if (bind(gSocket, (struct sockaddr *)&gAddress, sizeof(gAddress)) < 0) {
        printf("Bind failed\n");
        return;
    } else {
        printf("Binding...\n");
    }

    gHandler = handler;

    // Listen
    listen(gSocket, 10);
}

int getPort()
{
    socklen_t len = sizeof(gAddress);
    if(getsockname(gSocket, (struct sockaddr *)&gAddress, &len) == -1)
        printf("Could not getsockname\n");
    else
        return ntohs(gAddress.sin_port);

    return -1;
}

void StartListening()
{
    int clientSocket, sockaddrSize;
    struct sockaddr_in client;

    // Accept and incoming connection
    printf("Waiting for incoming connections...\n");

    sockaddrSize = sizeof(struct sockaddr_in);

    while((clientSocket = accept(gSocket, (struct sockaddr *)&client, (socklen_t*)&sockaddrSize)) )
    {
        pthread_t clientThread;
        int *newSocket = (int *)malloc(sizeof(int));
        *newSocket = clientSocket;

        printf("New client connected\n");

        if(pthread_create(&clientThread, NULL, gHandler, (void *)newSocket) < 0) {
            printf("Cannot create thread\n");
            return;
        }
    }
}

void *TrackingServerHandler(void *args) {
    int socket = *((int *)args);
    int recvSize;
    char buffer[MAX_LEN];

    while((recvSize = recv(socket, buffer, MAX_LEN, 0)) > 0) {
        buffer[recvSize] = '\0';

        // Handle requests from file server
        printf("%s\n", buffer);
    }   

    if(recvSize == 0) {
        printf("Client disconnected\n");
    } else if(recvSize == -1) {
        printf("Recv error\n");
    }

    // Free args
    free(args);

    // Close the socket
    close(socket);
    
    return NULL;
}

void *FileServerHandler(void *args) {
    int socket = *((int *)args);
    int recvSize;
    char buffer[MAX_LEN];

    while((recvSize = recv(socket, buffer, MAX_LEN, 0)) > 0) {
        buffer[recvSize] = '\0';

        // Handle requests from file server
        printf("%s\n", buffer);
    }   

    if(recvSize == 0) {
        printf("Client disconnected\n");
    } else if(recvSize == -1) {
        printf("Recv error\n");
    }

    // Free args
    free(args);

    // Close the socket
    close(socket);

    return NULL;
}

void InitTrackingServer(int port) {
    InitServer(port, TrackingServerHandler);
}

void InitFileServer(int port) {
    InitServer(port, FileServerHandler);
}

int ConnectToServer(char *serverIP, int serverPort) {
    struct sockaddr_in server;
    int serverSocket;

    // Create socket for connecting to server
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        printf("Socket creation failed\n");
        return serverSocket;
    }

    server.sin_addr.s_addr = inet_addr(serverIP);
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);

    // Connect to server
    if(connect(serverSocket, (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("Connection failed\n");
        return serverSocket;
    }

    return serverSocket;
}

int SendToSocket(int socket, char *buffer, int len) {
    return send(socket, buffer, len, 0);
}

int RecvFromSocket(int socket, char *buffer) {
    return recv(socket, buffer, MAX_LEN, 0);
}

void RecvACK(int socket) {
    char buffer[MAX_LEN];

    RecvFromSocket(socket, buffer);

    if (strncmp("ACK", buffer, strlen("ACK") != 0)) {
        printf("Error, expected ACK to be received: %s\n", buffer);
    }
}

void UpdateList(char *IP, int port, char *buffer, int len) {
    // Send the file list to tracking server
    // Prepare server socket
    int serverSocket = ConnectToServer(IP, port);

    // Send buffer to tracking server
    SendToSocket(serverSocket, buffer, len);

    // Wait for ACK
    RecvACK(serverSocket);
}
