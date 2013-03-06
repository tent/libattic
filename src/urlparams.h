#ifndef URLPARAMS_H_
#define URLPARAMS_H_
#pragma once

#include <map>
#include <vector>
#include <string>

class UrlParams
{
    typedef std::vector<std::string> UrlParam;
    typedef std::map<std::string, UrlParam> UrlParamMap;
public:
    UrlParams();
    ~UrlParams();
    void AddValue(const std::string& key, const std::string &value);
    UrlParam GetValue(const std::string& key);

    void SerializeToString(std::string &out) const;
    void SerializeAndEncodeToString(std::string &out) const;
private:
    UrlParamMap m_Values;
};

#endif

