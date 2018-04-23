#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>

using namespace std;

class Client{
    private:
        int id;
        int port;
        string ip;

        vector<string> fileList;

    public:
        Client(const int, const int, const string);
        int getID() const;
        int getPort() const;
        string getIP() const;

        vector<string> getFileList();

        int findFile(const string);

        void updateFileList(vector<string>);
};

#endif //CLIENT_H
