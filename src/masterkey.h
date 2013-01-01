

#ifndef MASTERKEY_H_
#define MASTERKEY_H_
#pragma once

#include <time.h>
#include <string>

#include "jsonserializable.h"
#include "crypto.h"

class MasterKey : JsonSerializable
{
public:
    MasterKey();
    ~MasterKey();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void WriteToFile(const std::string& filepath);
    void LoadFromFile(const std::string& filepath);

    void GenerateMasterKeyWithSentinel();

    void GetMasterKey(std::string& out)                     { m_Cred.GetKey(out); }
    void GetMasterKeyWithSentinel(std::string& out) const   { out = m_KeyWithSentinel; }
    time_t GetTimeCreated() const                           { return m_Created; }
    time_t GetExpires() const                               { return m_Expires; }
    
    void SetTimeCreated(const time_t time)          { m_Created = time; }
    void SetTimeExpires(const time_t time)          { m_Expires = time; }
    void SetCredentials(const Credentials& cred)    { m_Cred = cred; }

private:
    Credentials  m_Cred;
    std::string  m_KeyWithSentinel;
    time_t m_Created; // Time created
    time_t m_Expires; // Time set to expire by

};

#endif

