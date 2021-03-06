#include "Client.h"
#include "debug.h"
#include "file.h"
#include "tcp.h"
#include "func.h"
#include "stdio.h"
#include "hash.h"
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
int gServerSocket;
int gSocket;
char *machID;
char clientID[MAX_LEN];
void *(*gHandler)(void *);
static int gMachID = 0;
int gtracker_port;
char *gtracker_ip;

std::vector<Client*> gClientList;

std::multimap<char*, char*> fileMap;

void InitServer(int port, void *(*handler)(void *))
{
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
    int y = 1;
    if (setsockopt(gSocket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int))< 0) {
	printf("failed to set sock opt\n"); 
    }
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

char *getID()
{
   return clientID;
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

void *TrackingServerHandler(void *args) {
    // handles messages from the client
    printf("tracking server handler called \n");
    int socket = *((int *)args);
    int recvSize;
    char buffer[MAX_LEN];

    recvSize = recv(socket, buffer, MAX_LEN, 0);
    
    printf("recvd %s\n", buffer);


    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int result = getpeername(socket, (struct sockaddr *)&addr, &addr_size);

    int port = atoi(buffer);
    char *ip = inet_ntoa(addr.sin_addr);

    printf(" port %d my port is %d\n", port, getPort());
    ConnectToServer(ip, port, getPort());
    std::string ipString(ip);
    int connSock = atoi(strtok(buffer, ";"));
    
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
                printf("file %s\n", filename);
                std::string fn(filename);
                fileList.push_back(fn);
                filename = strtok(NULL, ";");
            }
            c->setID(machid);
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
    printf("started file server handler\n");
//    while (1) {
    recvSize = recv(socket, buffer, MAX_LEN, 0);
    printf("recvd %s\n", buffer);
    
    SendToSocket(socket, "ACK", strlen("ACK"));
    
    int trackerPort = atoi(buffer);
    printf("port %d\n", trackerPort);
   
    int connSock = atoi(strtok(buffer, ";"));
    printf("conn Sock %d\n", connSock);
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
                SendToSocket(socket, line, strlen(line));
                recvSize = RecvFromSocket(socket, buffer);
                buffer[recvSize] = '\0';
                msg = strtok(buffer, ";");
            }

            fclose(fp);

        } 
        else 
        {
            printf("Command not recognized\n");
        }
    }

        if(recvSize <= 0) {
            printf("Error: Client disconnected\n");
	    close(gServerSocket);
	    close(connSock);   
	    int my_port = getPort();
	    // reconnect in a loop with timeout
	    int sock;
	    while(1)
	    {
		printf("Sleeping\n");
	        // if tracking server disconnected, timeout for 0.5 seconds
	        usleep(5000000);
  		printf("ID %s\n", getID());
                char id[MAX_LEN];
	        memset(id, '\0', MAX_LEN);
	        sprintf(id, getID());
		sock = ConnectClientToServer(gtracker_ip, gtracker_port, my_port, id);
 	        if( sock >= 0)
	        {
		    break;
		}
	    } 
            
	} else if(recvSize == -1) {
            printf("Recv error\n");
        }
    //}

    // Free args
    free(args);

    // Close the socket
    close(socket);

    return NULL;
}

void InitTrackingServer(int port) {
    InitServer(port, TrackingServerHandler);
}

void InitFileServer(char *id, int port)
{
    machID = id;
    InitServer(port, FileServerHandler);
}

int ConnectToServer(char *serverIP, int serverPort, int clientPort)
{
    gtracker_port = serverPort;
    gtracker_ip = serverIP;
    struct sockaddr_in server; 

    // Create socket for connecting to server
    gServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (gServerSocket == -1) {
        printf("Socket creation failed\n");
        return gServerSocket;
    }

    server.sin_addr.s_addr = inet_addr(serverIP);
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);

    // Connect to server
    if(connect(gServerSocket, (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("Connection failed\n");
        return gServerSocket;
    }
    char buf[MAX_LEN];
    sprintf(buf, "%d;%d", clientPort, gSocket);
    SendToSocket(gServerSocket, buf, strlen(buf));
    return gServerSocket;
}

int ConnectClientToServer(char *server_ip, int server_port, int client_port, char *ID)
{
  memset(clientID, '\0', MAX_LEN);
  sprintf(clientID, ID);
  printf("clientID %s %s \n", clientID, ID);
  int i = ConnectToServer(server_ip, server_port, client_port);
  char buffer[MAX_LEN];
  memset(buffer, '\0', MAX_LEN);
  UpdateList(gServerSocket, buffer, ID);
  return i;
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
        printf("Error, expected ACK to be received: %s \n", buffer);
    }
}
