

#ifndef CRYPTO_H_
#define CRYPTO_H_
#pragma once

#include <string>
#include <fstream>

#include <aes.h>
#include <osrng.h>

#include "errorcodes.h"

struct Credentials
{
    byte key[CryptoPP::AES::MAX_KEYLENGTH];
    byte iv[CryptoPP::AES::BLOCKSIZE];
    
    size_t GetKeySize() const { return sizeof(byte) * CryptoPP::AES::MAX_KEYLENGTH; }
    size_t GetIvSize() const { return sizeof(byte) * CryptoPP::AES::BLOCKSIZE; } 
};

class Crypto
{
    bool EncryptData(const char* pData, unsigned int size, const Credentials &cred, std::ofstream &ofs);
    bool DecryptData(const char* pData, unsigned int size, const Credentials &cred, std::ofstream &ofs);

public:
    Crypto(unsigned int uStride = 400000);
    ~Crypto();

    Credentials GenerateCredentials(); 

    ret::eCode EncryptFile(const std::string &szFilepath, const std::string &szOutputPath, const Credentials &cred);
    ret::eCode DecryptFile(const std::string &szFilepath, const std::string &szOutputPath, const Credentials &cred);

    unsigned int GetStride() { return m_Stride; }

    void SetStride(unsigned int uStride) { m_Stride = uStride; }
    
private: 
    CryptoPP::AutoSeededRandomPool  m_Rnd; // Random pool used for key generation
    unsigned int                    m_Stride; // Size of stride used when encrypting

};


#endif

