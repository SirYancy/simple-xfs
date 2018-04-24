#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>

using namespace std;

class Client{

    friend bool operator==(const Client &c1, const Client &c2);
    friend bool operator!=(const Client &c1, const Client &c2);

    private:
        string id;
        int port;
        string ip;

        vector<string> fileList;

    public:
        Client(const int, const string);
        string getID() const;
        void setID(const string);
        int getPort() const;
        string getIP() const;

        vector<string> getFileList();

        int findFile(const string);

        void updateFileList(vector<string>);
};

#endif //CLIENT_H
