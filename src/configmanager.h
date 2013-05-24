#ifndef CONFIGMANAGER_H_
#define CONFIGMANAGER_H_
#pragma once

#include <map>
#include <string>

#include "mutexclass.h"

namespace attic { 

class ConfigManager : public MutexClass {
    ConfigManager();
    ConfigManager(const ConfigManager& rhs);
    ~ConfigManager();
    ConfigManager operator=(const ConfigManager& rhs);

public:
    static ConfigManager* GetInstance();

    void Shutdown();

    bool ValueExists(const std::string& key);
    bool GetValue(const std::string& key, std::string& out);
    void SetValue(const std::string& key, const std::string& value);

    std::string toString();
private:
    typedef std::map<std::string, std::string> ConfigMap;
    ConfigMap config_map_;

    static ConfigManager* instance_;
};

} //namespace
#endif 
