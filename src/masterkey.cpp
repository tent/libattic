#include "masterkey.h"

#include <fstream>

#include "utils.h"

MasterKey::MasterKey()
{

}

MasterKey::~MasterKey()
{

}

void MasterKey::Serialize(Json::Value& root)
{
    root["created"] = (int)m_Created;
    root["expires"] = (int)m_Expires;

    // Credentials
    root["key"] = m_Cred.GetKey();
    root["iv" ] = m_Cred.GetIv();    // This is obsolete, probably won't exist in the future
}

void MasterKey::Deserialize(Json::Value& root)
{
    m_Created = root.get("created", 0).asInt();
    m_Expires = root.get("expires", 0).asInt();

    std::string key = root.get("key", "").asString();
    std::string iv = root.get("iv", "").asString();

    if(!key.empty())
        m_Cred.SetKey(key);

    if(!iv.empty())
        m_Cred.SetIv(iv);
}

void MasterKey::WriteToFile(const std::string& filepath)
{
    std::ofstream ofs;

    ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);

    if(ofs.is_open())
    {
        // serialize
        std::string json;
        JsonSerializer::SerializeObject(this, json);
        
        // write out
        ofs.write(json.c_str(), json.size());

        ofs.close();
    }
}

void MasterKey::LoadFromFile(const std::string& filepath)
{
    std::ifstream ifs;

    ifs.open(filepath.c_str(), std::ios::in | std::ios::binary);

    if(ifs.is_open())
    {
        // TODO :: finish this

        ifs.close();
    }
}

bool MasterKey::InsertDirtyKey(const std::string& key)
{
    // sets and verifies master key from dirty key
    // Check sentinel bytes                                 
    std::string sentone, senttwo;                           
    sentone = key.substr(0, 4);                             
    senttwo = key.substr(4, 4);                             
    
    if(sentone != senttwo)                                  
        return false;

    m_KeyWithSentinel = key;
    // extract actual key apart from sentinel bytes     
    std::string keyActual;                              
    keyActual = key.substr(8);                          
    SetMasterKey(keyActual);
   
    return true;
}

void MasterKey::InsertSentinelIntoMasterKey()
{
    std::string key;
    GetMasterKey(key);

    std::string sent;
    utils::GenerateRandomString(sent, 4);

    sent += sent;

    m_KeyWithSentinel.clear();
    m_KeyWithSentinel += sent;
    m_KeyWithSentinel += key;
}

