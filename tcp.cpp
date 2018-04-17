#include "debug.h"
#include "tcp.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void InitServer(int port, void *(*handler)(void *)) {
    int serverSocket, clientSocket, sockaddrSize;
    struct sockaddr_in server, client;
    
    // Create socket
    serverSocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);

    if (serverSocket == -1) {
        debug_print("Could not create socket\n");
        return;
    } else {
        debug_print("Socket created with port %d\n", port);
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind
    if (bind(serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0) {
        debug_print("Bind failed\n");
        return
    } else {
        debug_print("Binding...\n");
    }

    // Listen
    listen(serverSocket , 3);

    // Accept and incoming connection
    printf("Waiting for incoming connections...\n");

    sockaddrSize = sizeof(struct sockaddr_in);

    while(clientSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&c)) {
        pthread_t clientThread;
        int *newSocket = (int *)malloc(sizeof(int));
        *newSocket = clientSocket;

        printf("New client connected\n");

        if(pthread_create(&clientThread, NULL, handler, (void *)newSocket) < 0) {
            debug_print("Cannot create thread\n");
            return;
        }
    }

    return;
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
        debug_print("Client disconnected\n");
    } else if(recvSize == -1) {
        debug_print("Recv error\n");
    }

    // Free args
    free(args);

    // Close the socket
    close(socket);
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
        debug_print("Client disconnected\n");
    } else if(recvSize == -1) {
        debug_print("Recv error\n");
    }

    // Free args
    free(args);

    // Close the socket
    close(socket);
}

void InitTrackingServer(int port) {
    InitServer(port, TrackingServerHanlder);
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
        debug_print("Socket creation failed\n");
        return serverSocket;
    }

    server.sin_addr.s_addr = inet_addr(serverIP);
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);

    // Connect to server
    if(connect(gServerSocket, (struct sockaddr *)&server , sizeof(server)) < 0) {
        debug_print("Connection failed\n");
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
        debug_print("Error, expected ACK to be received: %s\n", buffer);
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
