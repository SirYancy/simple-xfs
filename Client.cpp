#include <algorithm>
#include <vector>
#include <string>

#include "Client.h"

using namespace std;

Client::Client(const int machid, const int p, const string ip)
    :id(machid), port(p), ip(ip)
{}

int Client::getID() const
{
    return id;
}

int Client::getPort() const
{
    return port;
}

string Client::getIP() const
{
    return ip;
}

vector<string> Client::getFileList()
{
    return fileList;
}

int Client::findFile(const string filename)
{
    auto it = std::find(fileList.begin(), fileList.end(), filename);
    if(it != fileList.end())
        return 1;
    else
        return 0;
}

void Client::updateFileList(vector<string> fl)
{
    fileList = fl;
}

