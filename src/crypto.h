#ifndef CRYPTO_H_
#define CRYPTO_H_
#pragma once

#include <string>
#include <fstream>

#include <aes.h>
#include <osrng.h>

#include "errorcodes.h"
#include "credentials.h"

// TODO :: remove depricated methods

class Crypto
{
                      
    // Depricated
    bool EncryptData( const char* pData, 
                      unsigned int size, 
                      const Credentials &cred, 
                      std::ofstream &ofs);

    // Depricated
    bool DecryptData( const char* pData, 
                      unsigned int size, 
                      const Credentials &cred, 
                      std::ofstream &ofs);


public:
    Crypto(unsigned int uStride = 400000);
    ~Crypto();

    void GenerateCredentials(Credentials& cred);
    void GenerateIv(std::string& out);

    Credentials GenerateCredentials(); 

    ret::eCode EncryptFile( const std::string& filepath, 
                            const std::string& outputPath, 
                            const Credentials& cred);

    ret::eCode DecryptFile( const std::string& filepath, 
                            const std::string& outputPath, 
                            const Credentials& cred);

    int EncryptStringCFB( const std::string& data,
                          const Credentials& cred,
                          std::string& out);

    int DecryptStringCFB( const std::string& cipher,
                          const Credentials& cred,
                          std::string& out);

    int EncryptStringGCM( const std::string& data,
                          const Credentials& cred,
                          std::string& out);

    int DecryptStringGCM( const std::string& cipher,
                          const Credentials& cred,
                          std::string& out);

    int GenerateHMACForFile( const std::string& filepath,
                             const Credentials& cred,
                             std::string& macOut);
    
    int VerifyHMACForFile( const std::string& filepath,
                           const Credentials& cred,
                           const std::string& mac);

    int GenerateHMACForString( const std::string& input,
                               const Credentials& cred,
                               std::string& macOut);

    int VerifyHMACForString( const std::string& input,
                             const Credentials& cred,
                             const std::string& mac);



    unsigned int GetStride() { return m_Stride; }

    void SetStride(unsigned int uStride) { m_Stride = uStride; }

    int CheckSalt(std::string& salt);
    int GenerateSalt(std::string& out);

    int GenerateKeyFromPassphrase( const std::string &pass, 
                                   std::string &salt,
                                   Credentials& out);

    bool GenerateHash( const std::string& source, 
                       std::string& hashOut);
  
    bool ScryptEncode( const std::string &input, 
                       const std::string &salt,
                       std::string &out, 
                       unsigned int size);

   
private: 
    CryptoPP::AutoSeededRandomPool  m_Rnd; // Random pool used for key generation
    unsigned int                    m_Stride; // Size of stride used when encrypting
};


#endif

