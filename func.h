#ifndef FUNC_H
#define FUNC_H

#include "Client.h"

void FindFile(char *filename, vector<Client*>* clients, char *buffer);
void DownloadFile(char *IP, int port, char *buffer);
int GetLoad(char *IP, int port, char *buffer);
void UpdateList(char *IP, int port);

#endif //FUNC_H
