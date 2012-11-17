

#ifndef CRYPTO_H_
#define CRYPTO_H_
#pragma once

#include <string>
#include <fstream>

#include <aes.h>
#include <osrng.h>

struct Credentials
{
    byte key[CryptoPP::AES::MAX_KEYLENGTH];
    byte iv[CryptoPP::AES::BLOCKSIZE];
    
    size_t GetKeySize() { return sizeof(byte) * CryptoPP::AES::MAX_KEYLENGTH; }
    size_t GetIvSize() { return sizeof(byte) * CryptoPP::AES::BLOCKSIZE; } 
};

class Crypto
{
    bool EncryptData(const char* pData, unsigned int size, Credentials &cred, std::ofstream &ofs);
    bool DecryptData(const char* pData, unsigned int size, Credentials &cred, std::ofstream &ofs);

public:
    Crypto(unsigned int uStride = 400000);
    ~Crypto();

    Credentials GenerateCredentials(); 

    bool EncryptFile(std::string &szFilepath, std::string &szOutputPath, Credentials &cred);
    bool DecryptFile(std::string &szFilepath, std::string &szOutputPath, Credentials &cred);
    
private: 
    CryptoPP::AutoSeededRandomPool m_Rnd; // Random pool used for key generation
    unsigned int m_Stride; // Size of stride used when encrypting

};


#endif

