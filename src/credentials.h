#ifndef CREDENTIALS_H_
#define CREDENTIALS_H_
#pragma once

#include <string>
#include <stdio.h>
#include <aes.h>

#include "jsonserializable.h"

namespace attic { 

class Credentials : public JsonSerializable {
public:
    Credentials();
    ~Credentials();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void GetSerializedCredentials(std::string& out);

    std::string asString() const;

    size_t GetKeySize() const { return sizeof(byte) * CryptoPP::AES::MAX_KEYLENGTH; }
    size_t GetIvSize() const { return sizeof(byte) * CryptoPP::AES::BLOCKSIZE; } 

    const std::string& key() const { return key_; }
    const std::string& iv() const { return iv_; }

    const byte* byte_key() const { return byte_key_; }
    const byte* byte_iv() const { return byte_iv_; }

    int set_key(const std::string& key);
    int set_key(const byte* pKey, const unsigned int length);

    int set_iv(const std::string& iv); 
    int set_iv(const byte* pIv, const unsigned int length);

    bool byte_key_empty();
    bool byte_iv_empty();

    bool key_empty() const { return key_.empty(); }
    bool iv_empty() const { return iv_.empty(); } 

private:
    std::string key_;
    std::string iv_;

    byte byte_key_[CryptoPP::AES::MAX_KEYLENGTH+1];
    byte byte_iv_[CryptoPP::AES::BLOCKSIZE+1]; 
};

} //namespace
#endif

