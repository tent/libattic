
#ifndef URLVALUES_H_
#define URLVALUES_H_
#pragma once

#include <map>
#include <vector>
#include <string>


class UrlValues
{
    typedef std::vector<std::string> UrlValue;
    typedef std::map<std::string, UrlValue> UrlValueMap;
public:
    void AddValue(const std::string& key, const std::string &value);
    UrlValue GetValue(const std::string& key);

    std::string SerializeToString();

private:
    UrlValueMap m_Values;
};

#endif

