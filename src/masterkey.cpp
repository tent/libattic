#include "masterkey.h"

#include <fstream>
#include "utils.h"

void MasterKey::Serialize(Json::Value& root) {
    // Credentials
    root["key"] = credentials_.GetKey();
    root["iv" ] = credentials_.GetIv();    // This is obsolete, probably won't exist in the future
}

void MasterKey::Deserialize(Json::Value& root) {
    std::string key = root.get("key", "").asString();
    std::string iv = root.get("iv", "").asString();

    if(!key.empty())
        credentials_.SetKey(key);

    if(!iv.empty())
        credentials_.SetIv(iv);
}

void MasterKey::WriteToFile(const std::string& filepath) {
    std::ofstream ofs;

    ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);

    if(ofs.is_open()) {
        // serialize
        std::string json;
        jsn::SerializeObject(this, json);
        
        // write out
        ofs.write(json.c_str(), json.size());
        ofs.close();
    }
}

void MasterKey::LoadFromFile(const std::string& filepath) {
    std::ifstream ifs;

    ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if(ifs.is_open()) {
        // TODO :: finish this
        std::cout<<" master key load from file not implemented " << std::endl;
        ifs.close();
    }
}

bool MasterKey::InsertDirtyKey(const std::string& key) {
    // sets and verifies master key from dirty key
    // Check sentinel bytes                                 
    std::string sentone, senttwo;                           
    sentone = key.substr(0, 4);                             
    senttwo = key.substr(4, 4);                             
    
    if(sentone != senttwo)                                  
        return false;

    key_with_sentinel_ = key;
    // extract actual key apart from sentinel bytes     
    std::string keyActual;                              
    keyActual = key.substr(8);                          
    SetMasterKey(keyActual);
   
    return true;
}

void MasterKey::InsertSentinelIntoMasterKey() {
    std::string key;
    GetMasterKey(key);

    std::string sent;
    utils::GenerateRandomString(sent, 4);

    sent += sent;

    key_with_sentinel_.clear();
    key_with_sentinel_ += sent;
    key_with_sentinel_ += key;
}

