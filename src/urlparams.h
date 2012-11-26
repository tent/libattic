
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
    void AddValue(const std::string& key, const std::string &value);
    UrlParam GetValue(const std::string& key);

    std::string SerializeToString();

    static void UrlEncode(const std::string &url);
    static void UrlDecode(const std::string &url, std::string &out);

private:
    UrlParamMap m_Values;
};

#endif

