
#ifndef CREDENTIALS_H_
#define CREDENTIALS_H_
#pragma once

#include <string>
#include <stdio.h>
#include <aes.h>

#include "jsonserializable.h"

class Credentials : public JsonSerializable
{
public:
    Credentials();
    ~Credentials();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void GetSerializedCredentials(std::string& out);

    size_t GetKeySize() const { return sizeof(byte) * CryptoPP::AES::MAX_KEYLENGTH; }
    size_t GetIvSize() const { return sizeof(byte) * CryptoPP::AES::BLOCKSIZE; } 

    void GetKey(std::string& out) const { out.append(reinterpret_cast<const char*>(m_Key), GetKeySize()); }
    void GetIv(std::string& out) const { out.append(reinterpret_cast<const char*>(m_Iv), GetIvSize()); }

    int SetKey(const std::string& key);
    int SetIv(const std::string& iv); 

    bool KeyEmpty();
    bool IvEmpty();

    // TODO :: make this private
    byte m_Key[CryptoPP::AES::MAX_KEYLENGTH+1];
    byte m_Iv[CryptoPP::AES::BLOCKSIZE+1]; // TODO :: this is probably going to be removed in the future.
};


#endif

