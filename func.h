#include "Client.h"

char *FindFile(char *filename, vector<Client*>* clients);
void DownloadFile(char *IP, int port, char *buffer);
void GetLoad(char *IP, int port, char *buffer);
void UpdateList(char *IP, int port);
