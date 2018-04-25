#ifndef FUNC_H
#define FUNC_H

#include "Client.h"

void FindFile(char *filename, vector<Client*>* clients, char *buffer);
void DownloadFile(char *IP, int port, char *buffer);
void GetLoad(char *IP, int port, char *buffer);
void UpdateList(int socket, char *buffer, char *ID);

#endif //FUNC_H
