#include "tcp.h"
#include "file.h"
#include "func.h"

void FindFile(char *IP, int port, char *buffer) {
}

void DownloadFile(char *IP, int port, char *buffer) {
}

void GetLoad(char *IP, int port, char *buffer) {
    // Get the load from a specific file server
    int serverSocket;
    // char buffer[MAX_LEN];

    // Prepare server socket
    serverSocket = ConnectToServer(IP, port);
}

void UpdateList(char *IP, int port) {
    // Send the file list to tracking server
    int serverSocket, len;
    char buffer[MAX_LEN];

    // Prepare server socket
    serverSocket = ConnectToServer(IP, port);

    // Prepare file list
    len = GetFileList(buffer);

    // Send buffer to tracking server
    SendToSocket(serverSocket, buffer, len);

    // Wait for ACK
    RecvACK(serverSocket);
}
