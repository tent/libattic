#ifndef URLPARAMS_H_
#define URLPARAMS_H_
#pragma once

#include <map>
#include <vector>
#include <string>

namespace attic {

class UrlParams {
    typedef std::vector<std::string> UrlParam;
    typedef std::map<std::string, UrlParam> UrlParamMap;
public:
    UrlParams() {}
    ~UrlParams() {}
    void AddValue(const std::string& key, const std::string &value);
    UrlParam GetValue(const std::string& key);
    bool HasValue(const std::string& key);

    std::string asString();
    void SerializeToString(std::string& out) const;
    void SerializeAndEncodeToString(std::string& out) const;
    void DeserializeEncodedString(const std::string& in);
private:
    UrlParamMap values_;
};

}//namespace
#endif

