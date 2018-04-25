#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

#include "Client.h"

using namespace std;

Client::Client(const int p, const string ip)
    :port(p), ip(ip)
{}

string Client::getID() const
{
    return id;
}

void Client::setID(const string machid)
{
    id = machid;
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

