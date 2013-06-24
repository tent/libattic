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
    bool HasValue(const std::string& key) const;
    std::string GetValue(const std::string& key) const;
    void GetValue(const std::string& key, std::string& out) const;

    void ParseString(const std::string& in);
    void ReturnAsString(std::string& out) const ;

    void clear() { values_.clear(); }

    std::string asString() const;

private:
    HttpHeaderMap values_;
};

}//namespace
#endif

