#include "Client.h"
#include "debug.h"
#include "file.h"
#include "tcp.h"
#include "func.h"
#include "stdio.h"
#include "hash.h"
#include <time.h>
#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <map>

struct sockaddr_in gAddress;
int load = 1;
int latency = 0;
int gSocket;
char *machID;
void *(*gHandler)(void *);

std::vector<Client*> gClientList;

std::multimap<char*, char*> fileMap;

void InitServer(int port, void *(*handler)(void *))
{
    // initialize load to a non-zero value
    load = 1;
    // Create socket
    gSocket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP);

    if (gSocket == -1) {
        printf("Could not create socket\n");
        return;
    } else {
        printf("Socket created with port: %d \n", port);
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
        // for each connection received, start a new thread to handle messages from that client
        if(pthread_create(&clientThread, NULL, gHandler, (void *)newSocket) < 0) {
            printf("Cannot create thread\n");
            return;
        }
    }
}

void *TrackingServerHandler(void *args) 
{
    // handles messages from the client
    printf("tracking server handler called \n");
    int socket = *((int *)args);
    int recvSize;
    char buffer[MAX_LEN];

    recvSize = recv(socket, buffer, MAX_LEN, 0);

    printf("%s\n", buffer);

    SendToSocket(socket, "ACK", strlen("ACK"));

    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int result = getpeername(socket, (struct sockaddr *)&addr, &addr_size);

    int port = atoi(buffer);
    char *ip = inet_ntoa(addr.sin_addr);

    std::string ipString(ip);

    Client *c = new Client(port, ipString);
    gClientList.push_back(c);

    printf("Listening on new socket\n");
    while((recvSize = recv(socket, buffer, MAX_LEN, 0)) > 0) {
        buffer[recvSize] = '\0';

        // Handle requests from file server
        printf("buffer %s\n", buffer);

        char *command = strtok(buffer, ";");

        if (strcmp(command, "find") == 0)
        {
            char *filename = strtok(NULL, ";");

            FindFile(filename, &gClientList, buffer);

            SendToSocket(socket, buffer, strlen(buffer));
        } else if (strcmp(command, "register") == 0) {
            char *machid = strtok(NULL, ";");

            printf("registering: %s\n", machid);

            char *filename = strtok(NULL, ";");
            vector<string> fileList;
            while(filename != NULL) 
            {
                printf("file - %s\n", filename);
                std::string fn(filename);
                fileList.push_back(fn);
                filename = strtok(NULL, ";");
            }
            c->setID(machid);
            c->updateFileList(fileList);
        } else if (strcmp(command, "update") == 0) {
            printf("Updating file list for: %s\n", c->getID());

            char *filename = strtok(NULL, ";");
            vector<string> fileList;
            while(filename != NULL) 
            {
                printf("file - %s\n", filename);
                std::string fn(filename);
                fileList.push_back(fn);
                filename = strtok(NULL, ";");
            }
            c->updateFileList(fileList);
        } else {
            printf("Command not recognized\n");
        }

    }

    if(recvSize == 0) {
        printf("Client disconnected\n");
    } else if(recvSize == -1) {
        printf("Recv error\n");
    }

    // Removing this client from the list
    cout << "Erasing: " << c->getID() << endl;
    gClientList.erase(std::remove(gClientList.begin(), gClientList.end(), c), gClientList.end());

    // Free args
    free(args);

    // Close the socket
    close(socket);

    return NULL;
}

void *FileServerHandler(void *args) 
{
    addLoad();
    int socket = *((int *)args);
    int recvSize;
    char buffer[MAX_LEN];
    char *ack = "ACK";

    srand(time(NULL));

    recvSize = recv(socket, buffer, MAX_LEN, 0);
    SendToSocket(socket, ack, strlen(ack));

    printf("%s\n", buffer);

    while((recvSize = recv(socket, buffer, MAX_LEN, 0)) > 0) {
        buffer[recvSize] = '\0';
        //
        // Handle requests from file server
        printf("buffer: %s \n", buffer);

        char *command = strtok(buffer, ";");

        if (strcmp(command, "download") == 0)
        {
            char *filename = strtok(NULL, ";");

            std::string fn = "";

            fn.append(machID);
            fn.append("/");
            fn.append(filename);

            size_t checksum = get_hash(fn);

            snprintf(buffer, sizeof(buffer), "%zu", checksum);

            SendToSocket(socket, buffer, strlen(buffer));

            FILE *fp;
            ssize_t read;
            char *line = NULL;
            size_t len = 0;
            fp = fopen(fn.c_str(), "r");

            recvSize = RecvFromSocket(socket, buffer);
            buffer[recvSize] = '\0';
            char *msg = strtok(buffer, ";");

            while(strcmp(msg, "next") == 0)
            {
                read = getline(&line, &len, fp);
                if(read < 0)
                {
                    sprintf(buffer, "%s", "end");
                    SendToSocket(socket, buffer, strlen(buffer));
                    break;
                }

                // Introduce random errors for checksum testing 
                // 1% error on any line at some randome byte

                char outline[len];
                strcpy(outline, line);

                int r = rand() % 200;

                if(r == 0)
                {
                    printf("corrupting data\n");
                    int rc = rand() % len;
                    outline[rc] = 'X';
                }

                // Send this line of the file to the receiver
                SendToSocket(socket, outline, strlen(line));
                recvSize = RecvFromSocket(socket, buffer);
                buffer[recvSize] = '\0';
                msg = strtok(buffer, ";");
            } 

            fclose(fp);
        }
        else if (strcmp(command, "getload") == 0)
        {
            printf("current load: %d\n", load);
            sprintf(buffer, "%d", load);
            SendToSocket(socket, buffer, strlen(buffer));
        }
        else 
        {
            printf("Command not recognized\n");
        }
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

    subLoad();
    return NULL;
}

void InitTrackingServer(int port) 
{
    InitServer(port, TrackingServerHandler);
}

void InitFileServer(char *id, int port)
{
    machID = id;
    InitServer(port, FileServerHandler);
}

int ConnectToServer(char *serverIP, int serverPort, int clientPort)
{
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
    char buf[MAX_LEN];
    sprintf(buf, "%d", clientPort);
    SendToSocket(serverSocket, buf, strlen(buf));
    return serverSocket;
}

int SendToSocket(int socket, char *buffer, int len) {
    usleep(latency * 1000);     
    return send(socket, buffer, len, 0);
}

int RecvFromSocket(int socket, char *buffer) {
    return recv(socket, buffer, MAX_LEN, 0);
}

void RecvACK(int socket) {
    char buffer[MAX_LEN];

    RecvFromSocket(socket, buffer);

    if (strncmp("ACK", buffer, strlen("ACK") != 0)) {
        printf("Error, expected ACK to be received: %s \n", buffer);
    }
}

int getLoad()
{
    return load;
}
void addLoad()
{
    load++;
}
void subLoad()
{
    load--;
}

void setLatency(int lat)
{
    latency = lat;

}
