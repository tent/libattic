#include "configmanager.h"

namespace attic { 

ConfigManager* ConfigManager::instance_ = 0;

ConfigManager::ConfigManager(){}
ConfigManager::ConfigManager(const ConfigManager& rhs) {}
ConfigManager::~ConfigManager(){}
ConfigManager ConfigManager::operator=(const ConfigManager& rhs) { return *this;}

ConfigManager* ConfigManager::GetInstance() {
    if(!instance_)
        instance_ = new ConfigManager();
    return instance_;
}

void ConfigManager::Shutdown() {
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
}

bool ConfigManager::ValueExists(const std::string& key) {
    bool status = false;
    Lock();
    if(config_map_.find(key) != config_map_.end())
        status = true;
    Unlock();
    return status;
}

bool ConfigManager::GetValue(const std::string& key, std::string& out) {
    bool status = false;
    Lock();
    ConfigMap::iterator itr = config_map_.find(key);
    if(itr != config_map_.end()) {
        out = itr->second;
        status = true;
    }
    Unlock();
    return status;
}

void ConfigManager::SetValue(const std::string& key, const std::string& value) {
    Lock();
    config_map_[key] = value;
    Unlock();
}

std::string ConfigManager::toString() {
    std::string out;
    ConfigMap::iterator itr = config_map_.begin();
    for(;itr!=config_map_.end(); itr++) {
        out += itr->first;
        out += " : " ;
        out += itr->second;
        out += "\n";
    }
    return out;
}

} //namespace
