#ifndef MASTERKEY_H_
#define MASTERKEY_H_
#pragma once

#include <time.h>
#include <string>

#include "jsonserializable.h"
#include "crypto.h"

namespace attic { 

class MasterKey : JsonSerializable {
public:
    MasterKey() {}
    ~MasterKey() {}

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void WriteToFile(const std::string& filepath);
    void LoadFromFile(const std::string& filepath);

    bool InsertDirtyKey(const std::string& key); // sets and verifies master key from dirty key
    void InsertSentinelIntoMasterKey();

    const void GetMasterKey(std::string& out) const         { out = credentials_.key(); }
    const Credentials& credentials() const                  { return credentials_; }
    const std::string& key_with_sentinel() const            { return key_with_sentinel_; }
    
    void SetCredentials(const Credentials& cred)    { credentials_ = cred; }
    void SetMasterKey(const std::string& key)       { credentials_.set_key(key); } 
    void SetIv(const std::string& iv)               { credentials_.set_iv(iv); }
    

    bool IsEmpty() { return key_with_sentinel_.empty(); }

private:
    Credentials  credentials_;
    std::string  key_with_sentinel_;

};

}//namespace
#endif

