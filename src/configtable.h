#ifndef CONFIGTABLE_H_
#define CONFIGTABLE_H_
#pragma once

#include <string>
#include <deque>
#include "constants.h"
#include "tablehandler.h"

namespace attic { 

struct ConfigEntry {
    std::string type;
    std::string config_key;
    std::string value;
    std::string state;
};

class ConfigTable : public TableHandler {
    friend class Manifest;

    void ExtractEntryResults(const SelectResult& res, const int step, ConfigEntry& out);

    ConfigTable(sqlite3* db) : TableHandler(db, cnst::g_configtable) {}
    ConfigTable(const ConfigTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    ConfigTable operator=(const ConfigTable& rhs) { return *this; }
public:
    ~ConfigTable() {}

    bool CreateTable();
    
    bool InsertConfigValue(const std::string& type, 
                           const std::string& key, 
                           const std::string& value,
                           const std::string& state);
    bool RemoveConfigValue(const std::string& key);
    
    bool IsKeyInManifest(const std::string& key);
    bool IsValueInManifest(const std::string& value);
    bool IsStateInManifest(const std::string& state);

    bool DoesValueExist(const std::string& value);

    bool RetrieveConfigEntry(const std::string& key, ConfigEntry& out);
    bool RetrieveConfigValue(const std::string& key, std::string& out);
    bool RetrieveConfigValueByState(const std::string& state, std::string& out);
    bool RetrieveConfigKeyByState(const std::string& state, std::string& out);
    bool RetrieveConfigKeyByValue(const std::string& value, std::string& out);
    bool RetrieveConfigType(const std::string& type, std::deque<ConfigEntry>& out);

    bool RetrieveAllEntries(std::deque<ConfigEntry>& out);

    bool set_state_for_key(const std::string& key, const std::string& state);
    bool set_state_for_value(const std::string& value, const std::string& state);
};

}// namespace

#endif

