#include <iostream>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <unordered_map>
#include <fstream>
#include <limits>

#include "func.h"
#include "hash.h"
#include "tcp.h"

using namespace std;

struct serverDesc {
    char machid[20];
    char ip[20];
    int port;
};

unordered_map<string, int> latency_map;

int gServerSocket = 0;
int my_port = 0;
int tracker_port = 0;
char *tracker_ip;
char *myID;

int myLatency = 0;

char filename[25];

int live = 1;

void Init();

void parseLatency();
void register_client(int socket, char *buffer);
void find(int socket, char *buffer);
void download(int socket, char *buffer);
void updateList();
void readDirectory(char *buffer, char *folder);

bool dl_check(string fn, serverDesc s);
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

    // Set latency for all transactions
    parseLatency();
    setLatency(myLatency);

    InitFileServer(myID, 0, Init);

    my_port = getPort();

    cout << "Client starting on port " << my_port << endl;

    // Thread which listens for incoming connecitons
    pthread_create(&listenThread, NULL, listenerFunc, NULL);

    // UI Thread
    pthread_create(&clientThread, NULL, clientFunc, NULL);

    while(live){}

    pthread_join(listenThread, NULL);

    return 0;
}

/** Parses the latency.conf file so it knows
 * what every client's latency values are.
 */
void parseLatency()
{
    string fn = "latency.conf";
    ifstream infile(fn);
    string line;
    while(getline(infile, line))
    {
        char l[30];
        strcpy(l,line.c_str());

        char *id = strtok(l, ",");
        char *lat = strtok(NULL, ",");
        int lat_n = atoi(lat);
        if(strcmp(id, myID) == 0)
            myLatency = lat_n;
        else
            latency_map[id] = lat_n;
    }
}

/**
 * UI Thread function
 */
void *clientFunc(void *args)
{
    // Initialize and connect to tracker server
    Init();
    char buffer[MAX_LEN];
    memset(buffer, '\0', MAX_LEN); 

    int menu_choice;

    while(live)
    {
        memset(buffer, '\0', MAX_LEN);

        printf("Welcome, \n%s\n%s\n%s\n%s\n%s",
                "What do you want to do?",
                "1 - Find",
                "2 - Download",
                "3 - Exit",
                "> ");
        scanf("%d", &menu_choice);

        //flush input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }

        switch(menu_choice)
        {
            case 1:
                find(gServerSocket, buffer);
                break;
            case 2: 
                download(gServerSocket, buffer);
                break;
            case 3:
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

/**
 * Register with the tracker server
 */
void register_client(int socket, char *buffer)
{
    printf("registering client...\n");
    sprintf(buffer, "register;%s;", myID);
    char folder[20];
    strcpy(folder, myID);
    readDirectory(buffer, folder);

    SendToSocket(socket, buffer, strlen(buffer));
}

/**
 * Find a file on the tracker
 */
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

void printserver(serverDesc s)
{
    cout << s.machid << " " << s.ip << " " << s.port << endl;
}

/**
 * Find a file, get each server's latency, decide which
 * server to download from, and download.
 */
void download(int socket, char *buffer)
{
    addLoad();
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

    int currValue = numeric_limits<int>::max();
    serverDesc s;

    for(auto it = servers.begin(); it != servers.end(); ++it)
    {
        memset(buffer, '\0', MAX_LEN); 
        int load = GetLoad((*it).ip, (*it).port, buffer);
        int latency = latency_map[(*it).machid];
        int value = load * latency;

        cout << "load: " << load << " latency: " << latency << endl;

        if(value < currValue)
        {
            currValue = value;
            s = (*it);
        }
    }

    printf("Selected Server: %s, Value: %d\n", s.machid, currValue);

    string fn = "";

    fn.append(myID);
    fn.append("/");
    fn.append(filename);

    bool integrity = false;

    while(!integrity){
        integrity = dl_check(fn, s);
    }

    updateList();

    subLoad();
}

/**
 * Actually download the file and verify its hash.
 */
bool dl_check(string fn, serverDesc s)
{
    int len;
    char buffer[MAX_LEN];

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

    char *check = strtok(buffer, ";");
    char check_r[strlen(check)];
    strcpy(check_r, check);
    FILE *fp;
    remove(fn.c_str());
    fp = fopen(fn.c_str(), "a");

    while(buffer != NULL)
    {
        sprintf(buffer, "%s", "next;");
        SendToSocket(dlSocket, buffer, strlen(buffer));
        len = RecvFromSocket(dlSocket, buffer);
        buffer[len] = '\0';
        if(strcmp(buffer, "end") == 0)
        {
            break;
        }
        fprintf(fp, buffer);
    }

    close(dlSocket);
    fclose(fp);

    size_t check_n = get_hash(fn);

    char check_l[strlen(check_r)+1];
    snprintf(check_l, sizeof(check_l), "%zu", check_n);

    cout << "Remote Checksum: " << check_r << endl
        <<  "Local  Checksum: " << check_l << endl;

    if(strcmp(check_r, check_l) == 0)
    {
        cout << "Checksum Succeeded!" << endl;
        return true;
    }

    cout << "Checksum failed!" << endl;
    return false;
}

void updateList()
{
    //Update File List on tracker
    char buffer[MAX_LEN];
    sprintf(buffer, "update;");
    readDirectory(buffer, myID);
    SendToSocket(gServerSocket, buffer, strlen(buffer));
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
        // This blocks in the StartListening() function anyway.
    }
    return NULL;
}

void Init()
{
    char buffer[MAX_LEN];
    int trackerSocket = ConnectToServer(tracker_ip, tracker_port, my_port);
    gServerSocket = trackerSocket;

    // Wait for ACK from server
    int len = RecvFromSocket(trackerSocket, buffer);
    buffer[len] = '\0';
    printf("From server: %s\n", buffer);

    register_client(trackerSocket, buffer);
}
