
#ifndef CREDENTIALS_H_
#define CREDENTIALS_H_
#pragma once

#include <string>
#include <stdio.h>

#include <aes.h>

class Credentials
{
public:
    Credentials();
    ~Credentials();

    size_t GetKeySize() const { return sizeof(byte) * CryptoPP::AES::MAX_KEYLENGTH; }
    size_t GetIvSize() const { return sizeof(byte) * CryptoPP::AES::BLOCKSIZE; } 

    void GetKey(std::string& out) { out.append(reinterpret_cast<char*>(m_Key), GetKeySize()); }
    void GetIv(std::string& out) { out.append(reinterpret_cast<char*>(m_Iv), GetIvSize()); }

    int SetKey(const std::string& key);
    int SetIv(const std::string& iv); 

    // TODO :: make this private
    byte m_Key[CryptoPP::AES::MAX_KEYLENGTH];
    byte m_Iv[CryptoPP::AES::BLOCKSIZE]; // TODO :: this is probably going to be removed in the future.
};


#endif

