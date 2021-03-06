#ifndef TCP_H
#define TCP_H

#define MAX_LEN 1024

void InitTrackingServer(int port);
void InitFileServer(char* id, int port);

void StartListening();

int getPort();

int ConnectToServer(char *serverIP, int serverPort, int clientPort);
int ConnectClientToServer(char *serverIP, int serverPort, int clientPort, char* ID);
int SendToSocket(int socket, char *buffer, int len);
int RecvFromSocket(int socket, char *buffer);

void RecvACK(int socket);

void FindFile(char *IP, int port, char *buffer);
void DownloadFile(char *IP, int port, char *buffer);
void GetLoad(char *IP, int port, char *buffer);
void UpdateList(char *IP, int port);

#endif //TCP_H
