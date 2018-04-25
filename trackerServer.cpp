#include <iostream>
#include <pthread.h>
#include "tcp.h"

using namespace std;

int main(int argc, char* argv[]) {

    if(argc != 2)
    {
        cout << "Syntax is: ./tracker PORT_NUM" << endl;
        return 1;
    }

    int port = atoi(argv[1]);

    InitTrackingServer(port);

    cout << "Starting server on port " << port << endl;
    
    StartListening();

    return 0;
}

