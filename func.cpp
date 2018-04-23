#include "tcp.h"
#include "file.h"
#include "func.h"
#include "stdio.h"

char *FindFile(char *filename) {
    printf("trying to find file\n");
    //TODO Find filename. return list of servers with filename
}

void DownloadFile(char *IP, int port, char *buffer) {
}

void GetLoad(char *IP, int port, char *buffer) {
    // Get the load from a specific file server
    int serverSocket;
    // char buffer[MAX_LEN];

    // Prepare server socket
    serverSocket = ConnectToServer(IP, port, 0);
}

void UpdateList(char *IP, int port) {
    // Send the file list to tracking server
    int serverSocket, len;
    char buffer[MAX_LEN];

    // Prepare server socket
    serverSocket = ConnectToServer(IP, port, 0);

    // Prepare file list
    len = GetFileList(buffer);

    // Send buffer to tracking server
    SendToSocket(serverSocket, buffer, len);

    // Wait for ACK
    RecvACK(serverSocket);
}
