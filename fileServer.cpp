#include <iostream>
#include <pthread.h>
#include <string.h>
#include "tcp.h"
#include <dirent.h>

using namespace std;

int my_port = 0;
int tracker_port = 0;
char *tracker_ip;

int live = 1;

void readDirectory(char *buffer, char *folder);
void *listenerFunc(void *args);
void *clientFunc(void *args);

int main(int argc, char* argv[]) {

    pthread_t listenThread, clientThread;

    if(argc != 3)
    {
        cout << "Syntax is: ./tracker IP_ADDRESS PORT_NUM" << endl;
        return 1;
    }

    tracker_port = atoi(argv[2]);
    tracker_ip = argv[1];

    InitFileServer(0);

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
    int socket = ConnectToServer(tracker_ip, tracker_port);
    char buffer[MAX_LEN];

    int menu_choice;

    while(live)
    {
        memset(buffer, '\0', MAX_LEN);

        printf("Welcome, \n%s\n%s\n%s\n%s",
                "What do you want to do?",
                "1 - Exit",
                "2 - Register",
                "> ");
        scanf("%d", &menu_choice);

        //flush input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        switch(menu_choice)
        {
            case 1:
                printf("Exiting\n");
                live = 0;
                break;
            case 2: 
                char folder[MAX_LEN];
   		sprintf(folder, "./files");
		readDirectory(buffer, folder);
		SendToSocket(socket, buffer, strlen(buffer));
		break;
            default:
                printf("Invalid input\n");
                break;
        }
    }
    return NULL;
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
	    strcat (buffer, dir_entry->d_name);
	}
    printf("buffer %s\n", buffer);
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
    
    }
    return NULL;
}
