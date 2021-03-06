#include <iostream>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>

#include "hash.h"
#include "tcp.h"

using namespace std;

int my_port = 0;
int tracker_port = 0;
char *tracker_ip;
int load = 0;
char *myID;
char filename[25];

int live = 1;

void register_client(int socket, char *buffer);
void find(int socket, char *buffer);
void download(int socket, char *buffer);
void updateList(int socket, char *buffer);
void readDirectory(char *buffer, char *folder);

void *listenerFunc(void *args);
void *clientFunc(void *args);

int main(int argc, char* argv[]) {

    pthread_t listenThread, clientThread;

    if(argc != 4)
    {
        cout << "Syntax is: ./tracker IP_ADDRESS PORT_NUM FOLDER" << endl;
        return 1;
    }

    tracker_port = atoi(argv[2]);
    tracker_ip = argv[1];
    myID= argv[3];

    InitFileServer(myID, 0);

    my_port = getPort();

    cout << "Client starting on port " << my_port << endl;

    pthread_create(&listenThread, NULL, listenerFunc, NULL);
    pthread_create(&clientThread, NULL, clientFunc, NULL);

    while(live){}

    pthread_join(listenThread, NULL);

    return 0;
}

void *clientFunc(void *args)
{
    int trackerSocket = ConnectClientToServer(tracker_ip, tracker_port, my_port, myID);
    char buffer[MAX_LEN];
    memset(buffer, '\0', MAX_LEN); 

//    register_client(trackerSocket, buffer);

    int menu_choice;

    while(live)
    {
        memset(buffer, '\0', MAX_LEN);

        printf("Welcome, \n%s\n%s\n%s\n%s\n%s\n%s\n%s",
                "What do you want to do?",
                "1 - Find",
                "2 - Download",
                "3 - GetLoad",
                "4 - UpdateList",
                "5 - Exit",
                "> ");
        scanf("%d", &menu_choice);

        //flush input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }

        switch(menu_choice)
        {
            case 1:
                find(trackerSocket, buffer);
                break;
            case 2: 
                download(trackerSocket, buffer);
                break;
            case 3:
                // TODO Get load
                break;
            case 4:
                // TODO Upadte List
                break;
            case 5:
                printf("Exiting\n");
                live = 0;
                break;
            default:
                printf("Invalid input\n");
                break;
        }
    }
    return NULL;
}

void register_client(int socket, char *buffer)
{
    printf("registering client...\n");
    sprintf(buffer, "register;%s;", myID);
    char folder[20];
    strcpy(folder, myID);
    readDirectory(buffer, folder);

    SendToSocket(socket, buffer, strlen(buffer));
}

void find(int socket, char *buffer)
{
    printf("\nFilename: ");
    scanf("%[^\n]s", filename);

    // flush input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    sprintf(buffer, "find;%s", filename);

    SendToSocket(socket, buffer, strlen(buffer));

    int len = RecvFromSocket(socket, buffer);
    buffer[len] = '\0';

    cout << "Servers with file: " << filename <<
        endl << buffer << endl;
}

struct serverDesc {
    char machid[20];
    char ip[20];
    int port;
};

void printserver(serverDesc s)
{
    cout << s.machid << " " << s.ip << " " << s.port << endl;
}

void download(int socket, char *buffer)
{
    printf("\nFilename: ");
    scanf("%[^\n]s", filename);

    // flush input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    sprintf(buffer, "find;%s", filename);

    SendToSocket(socket, buffer, strlen(buffer));

    int len = RecvFromSocket(socket, buffer);
    buffer[len] = '\0';

    vector<serverDesc> servers;

    char *id = strtok(buffer, ";");
    while(id != NULL)
    {
        char *ip = strtok(NULL, ";");
        int port = atoi(strtok(NULL, ";"));

        serverDesc s;
        strcpy(s.machid, id);
        strcpy(s.ip, ip);
        s.port = port;

        servers.push_back(s);

        id = strtok(NULL, ";");
    }

    // TODO Get loads of servers to choose which server. For now, it's just going to download from the first one.

    serverDesc s = servers[0]; // TODO replace this
    printf("Connecting to server %s:%d\n", s.ip, s.port);
    int dlSocket = ConnectToServer(s.ip, s.port, my_port);

    len = RecvFromSocket(dlSocket, buffer);
    buffer[len] = '\0';
    printf("%s\n", buffer);

    memset(buffer, '\0', MAX_LEN); 
    sprintf(buffer, "download;%s;", filename);
    SendToSocket(dlSocket, buffer, strlen(buffer));

    len = RecvFromSocket(dlSocket, buffer);
    buffer[len] = '\0';

    char *checksum = strtok(buffer, ";");
    printf("Checksum: %s\n", checksum);

    char filedesc[MAX_LEN];
    sprintf(filedesc, "%s/%s", myID, filename);
    FILE *f;
    f = fopen(filedesc, "a");

    while(buffer != NULL)
    {
        sprintf(buffer, "%s", "next");
        SendToSocket(dlSocket, buffer, strlen(buffer));
        len = RecvFromSocket(dlSocket, buffer);
        buffer[len] = '\0';
        if(strcmp(buffer, "end") == 0)
        {
            break;
        }
        fprintf(f, buffer);
    }

    fclose(f);

    size_t my_checksum = get_hash(filedesc);
    printf("Checksum: %d\n", my_checksum);

    close(dlSocket);

}

void updateList(int socket, char *buffer)
{
    //TODO
}

void readDirectory(char* buffer, char* folder) 
{
    dirent* dir_entry;
    DIR* dir; 
    dir = opendir(folder);
    if (dir)
    {
        while (true)
        { 
            dir_entry = readdir(dir);
            if (dir_entry ==  NULL) break;
            else if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            { 
                continue;
            } 
            strcat (buffer, dir_entry->d_name);
            strcat (buffer, ";");
        }
        closedir(dir);
    }
    return;
}

void *listenerFunc(void *args)
{
    StartListening();

    cout << "Waiting for connections on listener socket..." << endl;
    while(live)
    {
        //Do whatever we should do. I assume this will just block at the loop in StartListening()
    }
    return NULL;
}
