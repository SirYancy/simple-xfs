#include <iostream>
#include <fstream>
#include <string>
#include "hash.h"

struct MyHash
{
    std::size_t operator()(std::string fn)
    {
        std::string line;
        std::ifstream fp(fn);

        std::size_t h = 1;
        while(std::getline(fp, line)) {
            std::size_t h1 = std::hash<std::string>{}(line);
            h = h ^ (h1 << 1); // Combine with next char
        }
        return h;
    }
};

std::size_t get_hash(std::string fn)
{
    return MyHash{}(fn);
}
