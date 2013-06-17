#include "configtable.h"
#include "logutils.h"

namespace attic {

bool ConfigTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (type TEXT, config_key TEXT, value TEXT, state TEXT,";
    exc += " PRIMARY KEY(type ASC, config_key ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("config_table_102482", error);
    return ret; 
}

bool ConfigTable::InsertConfigValue(const std::string& type, 
                                    const std::string& key, 
                                    const std::string& value, 
                                    const std::string& state) {
    bool ret = false;
    std::string exc;
    if(!IsConfigValueInManifest(key)) 
        exc += "INSERT OR REPLACE INTO ";
    else
        exc += "REPLACE INTO ";
    exc += table_name();
    exc += " (type, config_key, value, state) VALUES (?,?,?,?);";

    std::string error;
    ret = PrepareStatement(exc, error);     if(!ret) {log::ls("m_340s",error);return ret;}
    ret = BindText(1, type, error);         if(!ret) {log::ls("m_341s",error);return ret;}
    ret = BindText(2, key, error);          if(!ret) {log::ls("m_342s",error);return ret;}
    ret = BindText(3, value, error);        if(!ret) {log::ls("m_343s",error);return ret;}
    ret = BindText(4, state, error);        if(!ret) {log::ls("m_344s",error);return ret;}
    ret = StepStatement(error);             if(!ret) {log::ls("m_353s",error);return ret;}
    ret = FinalizeStatement(error);         if(!ret) {log::ls("m_354s",error);return ret;}
    return ret;
}

bool ConfigTable::RemoveConfigValue(const std::string& key) {
    bool ret = false;
    return ret;
}

bool ConfigTable::IsConfigValueInManifest(const std::string& key) {
    bool ret = false;
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE config_key=\"";
    query += key;
    query += "\");";
     
    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                std::string r = res.results()[0+step];
                if(r == "1") {
                    ret = true;
                    break;
                }
            }
        }
    }
    else {
        log::LogString("config_table_10941sm2s", error);
    }
    return ret;
}


bool ConfigTable::RetrieveConfigValue(const std::string& key, std::string& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE config_key=\"";
    query += key;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) { 
                out = res.results()[2+step];
                ret = true;
                break;
            }
        }
    }
    else {
        log::LogString("config_table_1045jhkdaf", error);
    }
    return ret;
}

bool ConfigTable::RetrieveConfigType(const std::string& type, std::deque<std::string>& out) {
    bool ret = false;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += " WHERE type=\"";
    query += type;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                out.push_back(res.results()[2+step]);
        }
        ret = true;
    }
    else {
        log::LogString("config_table_21014135", error);
    }
    return ret;
}

bool ConfigTable::RetrieveAllEntries(std::deque<ConfigEntry>& out) {
    bool ret = true;
    std::string query;
    query += "SELECT * FROM ";
    query += table_name();
    query += ";";

    std::string error;
    SelectResult res;
    if(Select(query, res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0) {
                ConfigEntry entry;
                ExtractEntryResults(res, step, entry);
                out.push_back(entry);
            }
        }
        ret = true;
    }
    else {
        log::LogString("config_table_122851", error);
    }
    return ret;
}

void ConfigTable::ExtractEntryResults(const SelectResult& res, const int step, ConfigEntry& out) {
    out.type = res.results()[0+step];
    out.config_key = res.results()[1+step];
    out.value = res.results()[2+step];
    out.state = res.results()[3+step];
}


} // namespace

