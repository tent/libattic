#ifndef HTTPHEADER_H_
#define HTTPHEADER_H_
#pragma once

#include <map>
#include <string>

namespace attic { 

class HttpHeader {
    typedef std::map<std::string, std::string> HttpHeaderMap;
public:
    HttpHeader() {}
    ~HttpHeader() {}

    std::string& operator[](const std::string& index);
    void AddValue(const std::string& key, const std::string& value);
    bool HasValue(const std::string& key);
    std::string GetValue(const std::string& key);
    void GetValue(const std::string& key, std::string& out);

    void ParseString(const std::string& in);
    void ReturnAsString(std::string& out);

    std::string asString(void);

private:
    HttpHeaderMap values_;
};

}//namespace
#endif

