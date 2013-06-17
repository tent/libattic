#ifndef CONFIGTABLE_H_
#define CONFIGTABLE_H_
#pragma once

#include <string>
#include <deque>
#include "constants.h"
#include "tablehandler.h"

namespace attic { 
namespace config {
    static const std::string dir_type("root_dir");
}

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
    
    bool IsConfigValueInManifest(const std::string& key);
    bool RetrieveConfigValue(const std::string& key, std::string& out);
    bool RetrieveConfigType(const std::string& type, std::deque<std::string>& out);

    bool RetrieveAllEntries(std::deque<ConfigEntry>& out);
};

}// namespace

#endif

