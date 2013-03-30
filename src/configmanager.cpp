#include "configmanager.h"

namespace attic { 

ConfigManager* ConfigManager::m_pInstance = 0;

ConfigManager::ConfigManager(){}
ConfigManager::ConfigManager(const ConfigManager& rhs) {}
ConfigManager::~ConfigManager(){}
ConfigManager ConfigManager::operator=(const ConfigManager& rhs) { return *this;}

ConfigManager* ConfigManager::GetInstance() {
    if(!m_pInstance)
        m_pInstance = new ConfigManager();
    return m_pInstance;
}

void ConfigManager::Shutdown() {
    if(m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

bool ConfigManager::ValueExists(const std::string& key) {
    bool status = false;
    Lock();
    if(m_ConfigMap.find(key) != m_ConfigMap.end())
        status = true;
    Unlock();
    return status;
}

bool ConfigManager::GetValue(const std::string& key, std::string& out) {
    bool status = false;
    Lock();
    ConfigMap::iterator itr = m_ConfigMap.find(key);
    if(itr != m_ConfigMap.end()) {
        out = itr->second;
        status = true;
    }
    Unlock();
    return status;
}

void ConfigManager::SetValue(const std::string& key, const std::string& value) {
    Lock();
    m_ConfigMap[key] = value;
    Unlock();
}

} //namespace
