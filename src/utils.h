
#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
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

};

#endif
