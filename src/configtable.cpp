#include "configtable.h"
#include "logutils.h"

namespace attic {

bool ConfigTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (config_key TEXT, value TEXT,";
    exc += " PRIMARY KEY(config_key ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("config_table_102482", error);
    return ret; 
}

bool ConfigTable::InsertConfigValue(const std::string& key, const std::string& value) {
    bool ret = false;
    return ret;
}

bool ConfigTable::RemoveConfigValue(const std::string& key) {
    bool ret = false;
    return ret;
}

bool ConfigTable::RetrieveSingleConfigValue(const std::string& key, std::string& out) {
    bool ret = false;
    return ret;
}

bool ConfigTable::RetrieveConfigValues(const std::string& key, std::deque<std::string>& out) {
    bool ret = false;
    return ret;
}

} // namespace
