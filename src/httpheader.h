#ifndef HTTPHEADER_H_
#define HTTPHEADER_H_
#pragma once

#include <map>
#include <string>

class HttpHeader
{
    typedef std::map<std::string, std::string> HttpHeaderMap;
public:
    HttpHeader();
    ~HttpHeader();

    void AddValue(const std::string& key, const std::string& value);
    bool ValueExists(const std::string& key);
    std::string GetValue(const std::string& key);
    void GetValue(const std::string& key, std::string& out);

    void ParseString(const std::string& in);
    void ReturnAsString(std::string& out);

private:
    HttpHeaderMap m_Values;
};


#endif

