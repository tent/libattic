#ifndef CONFIGPOST_H_
#define CONFIGPOST_H_
#pragma once

#include <string>
#include <map>

#include <post.h>

namespace attic { 

class ConfigPost : public Post {
public:
    ConfigPost();
    ~ConfigPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void PushBackConfigValue(const std::string& key, const std::string& value);

private:
    std::map<std::string, std::string> config_map_;
};

}// namespace
#endif

