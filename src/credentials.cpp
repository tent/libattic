#include "credentials.h"

#include <string.h>
#include "errorcodes.h"

Credentials::Credentials()
{
    memset(m_Key, '\0', CryptoPP::AES::MAX_KEYLENGTH+1);
    memset(m_Iv, '\0', CryptoPP::AES::BLOCKSIZE+1);

}

Credentials::~Credentials()
{

}

void Credentials::Serialize(Json::Value& root)
{
    std::string key, iv;

    GetKey(key);
    GetIv(iv);

    root["key"] = key;
    root["iv"] = iv;
}

void Credentials::Deserialize(Json::Value& root)
{
    std::string key, iv;

    key = root.get("key", "").asString();
    iv = root.get("iv", "").asString();

    SetKey(key);
    SetIv(iv);
}

void Credentials::GetSerializedCredentials(std::string& out)
{
    JsonSerializer::SerializeObject(this, out);
}

int Credentials::SetKey(const std::string& key) 
{ 
    int status = ret::A_OK;
    if(key.size() == GetKeySize())
    {
        memset(m_Key, '\0', CryptoPP::AES::MAX_KEYLENGTH+1);
        memcpy(m_Key, reinterpret_cast<const unsigned char*>(key.c_str()), GetKeySize());
    }
    else
    {
        std::cout<<" SIZE MISMATCH SIZE OF KEY : " <<  key.size() << std::endl;
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}

int Credentials::SetIv(const std::string& iv) 
{ 
    int status = ret::A_OK;
    if(iv.size() == GetIvSize())
    {
        memset(m_Iv, '\0', CryptoPP::AES::BLOCKSIZE+1);
        memcpy(m_Iv, reinterpret_cast<const unsigned char*>(iv.c_str()), GetIvSize());
    }
    else
    {
        status = ret::A_FAIL_IVSIZE_MISMATCH;
    }

    return status;
}

bool Credentials::KeyEmpty()
{
    if(strlen(reinterpret_cast<const char*>(m_Key)))
        return false;
    return true;
}

bool Credentials::IvEmpty()
{
    if(strlen(reinterpret_cast<const char*>(m_Iv)))
        return false;
    return true;
}
