#include <iostream>
#include <pthread.h>
#include <string.h>
#include "tcp.h"

using namespace std;

int my_port = 0;
int tracker_port = 0;
char *tracker_ip;

int live = 1;

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
    ConnectToServer(tracker_ip, tracker_port);
    char buffer[MAX_LEN];

    int menu_choice;

    while(live)
    {
        memset(buffer, '\0', MAX_LEN);

        printf("Welcome, \n%s\n%s\n%s",
                "What do you want to do?",
                "1 - Exit",
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
            default:
                printf("Invalid input\n");
                break;
        }
    }
    return NULL;
}

void *listenerFunc(void *args)
{
    StartListening();

    cout << "Waiting for connections on listener socket..." << endl;
    while(live)
    {
    
    }
}