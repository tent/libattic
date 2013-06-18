#ifndef CONFIGPOST_H_
#define CONFIGPOST_H_
#pragma once

#include <string>
#include <deque>

#include <post.h>

namespace attic { 

struct ConfigValue {
    std::string type;
    std::string key;
    std::string value;
};

class ConfigPost : public Post {
public:

    ConfigPost();
    ~ConfigPost();

    virtual void Serialize(Json::Value& root);  
    virtual void Deserialize(Json::Value& root);

    void PushBackConfigValue(const std::string& type, 
                             const std::string& key, 
                             const std::string& value);

    std::map<std::string, ConfigValue>* config_map() { return &config_map_; }

private:
    std::map<std::string, ConfigValue> config_map_;
};

}// namespace
#endif

