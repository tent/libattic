
#include "masterkey.h"

#include <fstream>

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
    root["key"] = m_Cred.key;
    root["iv" ] = m_Cred.iv;    // This is obsolete, probably won't exist in the future
}

void MasterKey::Deserialize(Json::Value& root)
{
    m_Created = root.get("created", 0).asInt();
    m_Expires = root.get("expires", 0).asInt();

    std::string key = root.get("key", "").asString();
    std::string iv = root.get("iv", "").asString();

    if(!key.empty())
    {
        memcpy(m_Cred.key, key.c_str(), key.size());
    }

    if(!iv.empty())
    {
        memcpy(m_Cred.iv, iv.c_str(), iv.size());
    }
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


