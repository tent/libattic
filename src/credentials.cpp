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
    jsn::SerializeObject(this, out);
}

int Credentials::SetKey(const std::string& key) 
{ 
    int status = ret::A_OK;

    if(key.size() <= GetKeySize())
    {
        memset(m_Key, '\0', GetKeySize()+1);
        memcpy(m_Key, key.c_str(), key.size());
    }
    else
    {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}

int Credentials::SetKey(const byte* pKey, const unsigned int length)
{
    int status = ret::A_OK;

    if(length <= GetKeySize())
    {
        memset(m_Key, '\0', GetKeySize()+1);
        memcpy(m_Key, pKey, length);
    }
    else
    {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}
int Credentials::SetIv(const std::string& iv) 
{ 
    int status = ret::A_OK;

    if(iv.size() <= GetIvSize())
    {
        memset(m_Iv, '\0', GetIvSize()+1);
        memcpy(m_Iv, iv.c_str(), iv.size());
    }
    else
    {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}
int Credentials::SetIv(const byte* pIv, const unsigned int length)
{
    int status = ret::A_OK;
    if(length <= GetIvSize())
    {
        memset(m_Iv, '\0', CryptoPP::AES::BLOCKSIZE+1);
        memcpy(m_Iv, pIv, length);
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

void Credentials::GetKey(std::string& out) const 
{ 
    out.append(reinterpret_cast<const char*>(m_Key), GetKeySize());
}

std::string Credentials::GetKey() const
{ 
    std::string out; 
    GetKey(out); 
    return out; 
}

void Credentials::GetIv(std::string& out) const 
{ 
    out.append(reinterpret_cast<const char*>(m_Iv), GetIvSize());
}

std::string Credentials::GetIv() const
{
    std::string out;
    GetIv(out);
    return out;
}

