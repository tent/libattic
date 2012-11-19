
#ifndef URLVALUES_H_
#define URLVALUES_H_
#pragma once

#include <map>
#include <string>

class UrlValues
{
public:
    void AddValue(std::string& key, std::string &value);
    std::string GetValue(std::string& key);

    std::string SerializeToString();

private:
    std::map<std::string, std::string> m_Values;
};

#endif

