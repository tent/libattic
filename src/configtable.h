#ifndef CONFIGTABLE_H_
#define CONFIGTABLE_H_
#pragma once

#include <string>
#include "constants.h"
#include "tablehandler.h"

namespace attic { 

class ConfigTable : public TableHandler {
    friend class Manifest;

    ConfigTable(sqlite3* db) : TableHandler(db, cnst::g_foldertable) {}
    ConfigTable(const FolderTable& rhs) : TableHandler(rhs.db(), rhs.table_name()) {}
    ConfigTable operator=(const FolderTable& rhs) { return *this; }
public:
    ~ConfigTable() {}

    bool CreateTable();
    
    bool InsertConfigValue(const std::string& type, const std::string& key, const std::string& value);
    bool RemoveConfigValue(const std::string& key);
    
    bool IsConfigValueInManifest(const std::string& key);
    bool RetrieveConfigValue(const std::string& key, std::string& out);
    bool RetrieveConfigType(const std::string& type, std::deque<std::string>& out);
};

}// namespace

#endif

