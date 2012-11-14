
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace utils
{

static std::vector<std::string> &SplitString(const std::string &s, char delim, std::vector<std::string> &out)
{
    std::stringstream ss(s);
    std::string item;

    while(std::getline(ss, item, delim))
    {
        out.push_back(item);
    }

    return out;
}

static unsigned int CheckFileSize(std::string &szFilePath)
{
    unsigned int fileSize = 0;

    std::ifstream ifs;
    ifs.open (szFilePath.c_str(), std::ifstream::binary);

    if(ifs.is_open())
    {
       ifs.seekg (0, std::ifstream::end);
       fileSize = ifs.tellg();
       ifs.seekg (0, std::ifstream::beg);
       ifs.close();
    }

    return fileSize;
}

};

#endif
