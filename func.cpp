#include "Client.h"
#include "tcp.h"
#include "file.h"
#include "func.h"
#include "stdio.h"
#include <string>

char *FindFile(char *filename, vector<Client*>* clients)
{
    printf("trying to find file\n");
    std::string fn(filename);
    std::string serverList;

    for (auto it = clients->begin(); it != clients->end(); ++it)
    {
        if((*it)->findFile(fn))
        {
            std::string ip = (*it)->getIP();
            int port = (*it)->getPort();
            serverList.append(ip);
            serverList.append(";");
            serverList.append(std::to_string(port));
            serverList.append(";");
        }
    }

    return const_cast<char*>(serverList.c_str());

}

void DownloadFile(char *IP, int port, char *buffer)
{

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
