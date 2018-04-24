#include <iostream>
#include <fstream>
#include <string>
#include "hash.h"

using namespace std;

struct MyHash
{
    size_t operator()(string fn)
    {
        string line;
        ifstream fp(fn);

        size_t h = 1;
        while(getline(fp, line)) {
            size_t h1 = hash<string>{}(line);
            h = h ^ (h1 << 1); // Combine with next char
        }
        return h;
    }
};

size_t get_hash(string fn)
{
    return MyHash{}(fn);
}
