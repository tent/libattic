#include "chunktable.h"

#include "logutils.h"

namespace attic { 

bool ChunkTable::CreateTable() {
    std::string exc;
    exc += "CREATE TABLE IF NOT EXISTS ";
    exc += table_name();
    exc += " (digest TEXT, plaintext_mac TEXT,";
    exc += " PRIMARY KEY(digest ASC, plaintext_mac ASC));";
    std::string error;
    bool ret = Exec(exc, error);
    if(!ret)
        log::LogString("manifest_09214", error);
    return ret; 
}

bool ChunkTable::InsertChunkDigest(const std::string& digest, const std::string& plaintext_mac) {
    std::string query;
    query += "INSERT OR REPLACE INTO ";
    query += table_name();
    query += " (digest, plaintext_mac)";
    query += " VALUES (?, ?);";

    std::string error;
    bool ret = false;
    ret = PrepareStatement(query, error);       if(!ret) {log::ls("m_340s",error);return ret;}
    ret = BindText(1, digest, error);           if(!ret) {log::ls("m_341s",error);return ret;}
    ret = BindText(2, plaintext_mac, error);    if(!ret) {log::ls("m_342s",error);return ret;}
    ret = StepStatement(error);                 if(!ret) {log::ls("m_343s",error);return ret;}
    ret = FinalizeStatement(error);             if(!ret) {log::ls("m_344s",error);return ret;}
    return ret;
}

bool ChunkTable::IsDigestInManifest(const std::string& digest) {
    bool ret = false;
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE digest=\"";
    query += digest;
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
        log::LogString("manifest_a2390_3jgs", error);
    }
    return ret;
}

bool ChunkTable::IsPlaintextMacInManifest(const std::string& plaintext_mac) {
    bool ret = false;
    std::string query;
    query += "SELECT EXISTS(SELECT * FROM ";
    query += table_name();
    query += " WHERE plaintext_mac=\"";
    query += plaintext_mac;
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
        log::LogString("manifest_201_53jgs", error);
    }
    return ret;
}

bool ChunkTable::GetChunkDigest(const std::string& plaintext_mac, std::string& out) {
    bool ret = false;
    std::string query;
    query += "SELECT digest FROM ";
    query += table_name();
    query += " WHERE plaintext_mac=\"";
    query += plaintext_mac;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query.c_str(), res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                out = res.results()[0+step];
        }
        ret = true;
    }
    else {
        log::LogString("manifest_157ppoj", error);
    }
    return ret;
}

bool ChunkTable::GetPlaintextMac(const std::string& digest, std::string& out) {
    bool ret = false;
    std::string query;
    query += "SELECT plaintext_mac FROM ";
    query += table_name();
    query += " WHERE digest=\"";
    query += digest;
    query += "\";";

    std::string error;
    SelectResult res;
    if(Select(query.c_str(), res, error)) {
        int step = 0;
        for(int i=0; i<res.row()+1; i++) {
            step = i*res.col();
            if(step > 0)
                out = res.results()[0+step];
        }
        ret = true;
    }
    else {
        log::LogString("manifest_18451ppoj", error);
    }
    return ret;
}


} //namespace

