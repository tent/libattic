#ifndef CRYPTO_H_
#define CRYPTO_H_
#pragma once

#include <string>
#include <fstream>

#include <hex.h>         
#include <filters.h>     
#include <gcm.h>         
#include <hmac.h>
#include <aes.h>
#include <osrng.h>
#include <base64.h>

#include "errorcodes.h"
#include "credentials.h"
#include "utils.h"

extern "C"
{
    #include "crypto_scrypt.h"
    int crypto_scrypt( const uint8_t *,
                       size_t,
                       const uint8_t *,
                       size_t,
                       uint64_t,
                       uint32_t,
                       uint32_t,
                       uint8_t *,
                       size_t);
}

static const int TAG_SIZE = 16;
static const int SALT_SIZE = 16;
static CryptoPP::AutoSeededRandomPool  g_Rnd;                // Random pool used for key generation
static unsigned int                    g_Stride = 400000;    // Size of stride used when encrypting

namespace crypto
{

    static Credentials GenerateCredentials();
    static void GenerateCredentials(Credentials& cred);
    static void GenerateIv(std::string& out);
    static bool GenerateHash( const std::string& source, std::string& hashOut);

    static int EncryptStringCFB( const std::string& data,
                                 const Credentials& cred,
                                 std::string& out);
 
    static int DecryptStringCFB( const std::string& cipher,
                                 const Credentials& cred,
                                 std::string& out);

    static int EncryptStringGCM( const std::string& data,
                                 const Credentials& cred,
                                 std::string& out);

    static int DecryptStringGCM( const std::string& cipher,
                                 const Credentials& cred,
                                 std::string& out);

    static int GenerateKeyFromPassphrase( const std::string& pass, 
                                          std::string& salt,
                                          Credentials& out);

    static bool ScryptEncode( const std::string &input, 
                              const std::string &salt,
                              std::string &out,
                              unsigned int size);

    static int CheckSalt(std::string& salt);
    static int GenerateSalt(std::string& out);

    static int GenerateHMACForString( const std::string& input,
                                      const Credentials& cred,
                                      std::string& macOut);

    static int VerifyHMACForString( const std::string& input,
                                    const Credentials& cred,
                                    const std::string& mac);


    
    static Credentials GenerateCredentials()
    {
        // This is returning a copy on purpose, 
        // When this is called, credentials should 
        // be used, then stored (if needed), and fall 
        // out of scope as quickly as possible
        Credentials cred;
        GenerateCredentials(cred);
        return cred;
    }

    static void GenerateCredentials(Credentials& cred)
    {
        byte key[CryptoPP::AES::MAX_KEYLENGTH+1];
        byte iv[CryptoPP::AES::BLOCKSIZE+1]; 

        g_Rnd.GenerateBlock( key, cred.GetKeySize());   // Generate a random key
        g_Rnd.GenerateBlock( iv, cred.GetIvSize());     // Generate a random IV

        cred.SetKey(key, CryptoPP::AES::MAX_KEYLENGTH);
        cred.SetIv(iv, CryptoPP::AES::BLOCKSIZE);
    }

    static void GenerateIv(std::string& out)
    {
        byte iv[CryptoPP::AES::BLOCKSIZE];
        memset(iv, 0, CryptoPP::AES::BLOCKSIZE);
        g_Rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE); 

        out.append(reinterpret_cast<char*>(iv), CryptoPP::AES::BLOCKSIZE);
    }

    static bool GenerateHash( const std::string& source, std::string& hashOut)
    {
        CryptoPP::SHA256 hash;

        CryptoPP::StringSource src( source.c_str(), 
                                    true,
                                    new CryptoPP::HashFilter( hash,
                                        new CryptoPP::Base64Encoder (
                                            new CryptoPP::StringSink(hashOut)
                                            )
                                        )
                                 );
        return true;
    }

    static int EncryptStringCFB( const std::string& data,
                                 const Credentials& cred,
                                 std::string& out)
    {
        int status = ret::A_OK;

        try {
            std::string cipher;

            CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e;
            e.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

            CryptoPP::StringSource( data,  // Plaintext
                                    true, 
                                    new CryptoPP::StreamTransformationFilter( e,
                                        new CryptoPP::StringSink(out)
                                    ) // StreamTransformationFilter      
                                  ); // StringSource
        }
        catch (CryptoPP::Exception &e) {
            status = ret::A_FAIL_ENCRYPT;
        }

        return status;
    }

    static int DecryptStringCFB( const std::string& cipher,
                                 const Credentials& cred,
                                 std::string& out)
    {
        int status = ret::A_OK;

        try {
            CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d;        
            d.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

            CryptoPP::StringSource( cipher, 
                                    true, 
                                    new CryptoPP::StreamTransformationFilter( d,
                                        new CryptoPP::StringSink( out )
                                             ) // StreamTransformationFilter
                                   ); // StringSource
        }
        catch (CryptoPP::Exception &e) {
            status = ret::A_FAIL_DECRYPT;
        }                                                            

        return status;
    }

    static int EncryptStringGCM( const std::string& data,
                                 const Credentials& cred,
                                 std::string& out)
    {
        int status = ret::A_OK;
        try {
            CryptoPP::GCM<CryptoPP::AES>::Encryption e;
            e.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

            CryptoPP::StringSource( data,
                                    true,
                                    new CryptoPP::AuthenticatedEncryptionFilter( e,
                                    new CryptoPP::StringSink(out),
                                    false,
                                    TAG_SIZE)
                                  );
        }
        catch (CryptoPP::Exception &e) {
            status = ret::A_FAIL_ENCRYPT;
        }

        return status;
    }

    static int DecryptStringGCM( const std::string& cipher,
                                 const Credentials& cred,
                                 std::string& out)
    {
        try {
            CryptoPP::GCM<CryptoPP::AES>::Decryption d;        
            d.SetKeyWithIV(cred.m_Key, cred.GetKeySize(), cred.m_Iv, cred.GetIvSize());

            // Recovered Plain Data                                                             
            std::string rpdata;                                                                 
            CryptoPP::AuthenticatedDecryptionFilter df ( d,                                              
                                                         new CryptoPP::StringSink(rpdata),
                                                         CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 
                                                         TAG_SIZE          
                                                       );                                                             
            CryptoPP::StringSource( cipher,                                                      
                                    true,                                                 
                                    new CryptoPP::Redirector( df /*  , PASS_EVERYTHING */ )
                                  ); // StringSource

            // If the object does not throw, here's the only                 
            //  opportunity to check the data's integrity                    
            if (df.GetLastResult() == true )                                 
            {                                                                
                //std::cout<< "recovered text : " << rpdata << "\n";           
                // Write out data to ofstream
                out = rpdata;
            }                                                                

        }
        catch (CryptoPP::Exception &e) {
            //std::cerr << e.what() << "\n";                           
            return ret::A_FAIL_DECRYPT;
        }                                                            
        return ret::A_OK;
    }



    static int GenerateKeyFromPassphrase( const std::string& pass, 
                                          std::string& salt,
                                          Credentials& out)
    {
        int status = ret::A_OK;
        std::string outKey; //, outIv;

        // Check salt for size and correctness
        status = CheckSalt(salt);
        if(status == ret::A_OK) {
            ScryptEncode(pass, salt, outKey, CryptoPP::AES::MAX_KEYLENGTH);
            //ScryptEncode(name, outIv, CryptoPP::AES::BLOCKSIZE);
            memcpy(out.m_Key, outKey.c_str(), CryptoPP::AES::MAX_KEYLENGTH);
            memcpy(out.m_Iv, salt.c_str(), salt.size());
        }

        return status;
    }

    static bool ScryptEncode( const std::string &input, 
                              const std::string &salt,
                              std::string &out,
                              unsigned int size)
    {
        // Note* pass in 16 bit salt
        //uint8_t salt[32]; // 16 <- do 16, 64 or 128

        uint8_t* password;
        size_t plen;

        uint64_t N = 16384;
        uint32_t r = 8;
        uint32_t p = 1;

        //uint8_t dk[64]; // Derived key
        uint8_t dk[size]; // Derived key

        byte* pInput = new byte[input.size()];
        memcpy(pInput, reinterpret_cast<const unsigned char*>(input.c_str()), input.size());

        byte* pSalt = new byte[salt.size()];
        memcpy(pSalt, reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size());

        crypto_scrypt( pInput,
                       input.size(),
                       pSalt,
                       salt.size(),
                       N,
                       r,
                       p,
                       dk,
                       size);
        

        out.append( reinterpret_cast<char*>(dk), sizeof(uint8_t)*size);

        if(pInput) {
            delete[] pInput;
            pInput = NULL;
        }

        if(pSalt) {
            delete[] pSalt;
            pSalt = NULL;
        }

        return true;
    }

    static int CheckSalt(std::string& salt)
    {
        int status = ret::A_OK;

        // Check for correct size
        if(salt.size() != SALT_SIZE) {
            status = ret::A_FAIL_SCRYPT_INVALID_SALT_SIZE;
        }
        return status;
    }

    static int GenerateSalt(std::string& out) 
    {
        int status = ret::A_OK;
        utils::GenerateRandomString(out, SALT_SIZE);
        return status;
    }

    static int GenerateHMACForString( const std::string& input,
                                      const Credentials& cred,
                                      std::string& macOut)
    {
        int status = ret::A_OK;
        std::string mac;
        try {
            CryptoPP::HMAC<CryptoPP::SHA256> hmac(cred.m_Key, cred.GetKeySize());
            CryptoPP::StringSource( input,
                                    true,
                                    new CryptoPP::HashFilter( hmac,
                                        new CryptoPP::StringSink(mac)
                                        )
                                  );
        }
        catch(const CryptoPP::Exception& e) {
            // Log error 
            std::cout << e.what() << std::endl;
            status = ret::A_FAIL_HMAC;
        }

        if(status == ret::A_OK) {
            std::string hexencoded;
            // Encode to hex
            CryptoPP::StringSource( mac, 
                          true,
                          new CryptoPP::HexEncoder(
                             new CryptoPP::StringSink(hexencoded)
                             ) // HexEncoder
                         ); // StringSource

            if(!hexencoded.empty())
                macOut = hexencoded;
            else
                status = ret::A_FAIL_HEX_ENCODE;
        }

        return status;
    }

    static int VerifyHMACForString( const std::string& input,
                                    const Credentials& cred,
                                    const std::string& mac)

    {
        int status = ret::A_OK;

        try {
            // Decode hmac
            std::string decoded;
            CryptoPP::StringSource ss( mac,
                                       true,
                                       new CryptoPP::HexDecoder(
                                            new CryptoPP::StringSink(decoded)
                                ) // HexDecoder
            ); // StringSource

            if(!decoded.empty()) {

                CryptoPP::HMAC<CryptoPP::SHA256> hmac(cred.m_Key, cred.GetKeySize());

                const int flags = CryptoPP::HashVerificationFilter::THROW_EXCEPTION | CryptoPP::HashVerificationFilter::HASH_AT_END;
            
                CryptoPP::StringSource( input + decoded, 
                                        true, 
                                        new CryptoPP::HashVerificationFilter(hmac, NULL, flags)
                                       ); // StringSource
            }
            else {
                status = ret::A_FAIL_HEX_DECODE;
            }
        }
        catch(const CryptoPP::Exception& e) {
            std::cout << e.what() << std::endl;
            status = ret::A_FAIL_HMAC_VERIFY;
        }

        return status;
    }


    static void Base64EncodeString(const std::string& input, std::string& output)
    {
        CryptoPP::StringSource( input, 
                                true, 
                                new CryptoPP::Base64Encoder(new CryptoPP::StringSink(output)));

    }

    static void Base64DecodeString(const std::string& input, std::string& output)
    {
        CryptoPP::StringSource( input,
                                true,
                                new CryptoPP::Base64Decoder(new CryptoPP::StringSink(output)));
    }



};


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

