#ifndef TCP_H
#define TCP_H

#define MAX_LEN 1024

void InitTrackingServer(int port);
void InitFileServer(char *id, int port, void (*callback)());

void StartListening();

int getPort();

int getLoad();

void addLoad();
void subLoad();

void setLatency(int lat);

int ConnectToServer(char *serverIP, int serverPort, int clientPort);

int SendToSocket(int socket, char *buffer, int len);
int RecvFromSocket(int socket, char *buffer);

void RecvACK(int socket);

#endif //TCP_H
