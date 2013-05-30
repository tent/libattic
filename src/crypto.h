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
#include <base32.h>
#include <base64.h>
#include <files.h>

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

namespace attic { namespace crypto {

static const int TAG_SIZE = 16;
static const int SALT_SIZE = 16;
static CryptoPP::AutoSeededRandomPool  g_Rnd;                // Random pool used for key generation
static unsigned int                    g_Stride = 400000;    // Size of stride used when encrypting


static Credentials GenerateCredentials();
static void GenerateCredentials(Credentials& cred);
static void GenerateIv(std::string& out);
static void GenerateSha256Hash(const std::string& source, std::string& hash_out);
static bool GenerateHash( const std::string& source, std::string& hash_out);
static bool GenerateHexEncodedHmac(const std::string& source, std::string& hash_out);
static void GenerateFileHash(const std::string& filepath, std::string& hash_out);
static void GenerateRandomString(std::string& out, const unsigned int size);

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
                                      const std::string& salt,
                                      Credentials& out);

static bool ScryptEncode( const std::string &input, 
                          const std::string &salt,
                          std::string &out,
                          unsigned int size);

static int CheckSalt(const std::string& salt);
static int GenerateSalt(std::string& out);

static int GenerateHMACForString(const std::string& input,
                                 const Credentials& cred,
                                 std::string& macOut);

static int VerifyHMACForString(const std::string& input,
                               const Credentials& cred,
                               const std::string& mac);

static Credentials GenerateCredentials() {
    // This is returning a copy on purpose, 
    // When this is called, credentials should 
    // be used, then stored (if needed), and fall 
    // out of scope as quickly as possible
    Credentials cred;
    GenerateCredentials(cred);
    return cred;
}

static void GenerateCredentials(Credentials& cred) {
    byte key[CryptoPP::AES::MAX_KEYLENGTH+1];
    byte iv[CryptoPP::AES::BLOCKSIZE+1]; 

    g_Rnd.GenerateBlock(key, cred.GetKeySize());   // Generate a random key
    g_Rnd.GenerateBlock(iv, cred.GetIvSize());     // Generate a random IV

    cred.set_key(key, CryptoPP::AES::MAX_KEYLENGTH);
    cred.set_iv(iv, CryptoPP::AES::BLOCKSIZE);
}

static void GenerateIv(std::string& out) {
    byte iv[CryptoPP::AES::BLOCKSIZE];
    memset(iv, 0, CryptoPP::AES::BLOCKSIZE);
    g_Rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE); 
    out.append(reinterpret_cast<char*>(iv), CryptoPP::AES::BLOCKSIZE);
}

static void GenerateSha256Hash(const std::string& source, std::string& hash_out) {
    CryptoPP::SHA256 hash;
    CryptoPP::StringSource src(source.c_str(), 
                               true,
                               new CryptoPP::HashFilter( hash,
                                   new CryptoPP::Base64Encoder (
                                       new CryptoPP::StringSink(hash_out),
                                       false
                                       )
                                   )
                              );
}

static bool GenerateHash(const std::string& source, std::string& hash_out) {
    CryptoPP::SHA512 hash;

    CryptoPP::StringSource src(source.c_str(), 
                               true,
                               new CryptoPP::HashFilter( hash,
                                   new CryptoPP::Base64Encoder (
                                       new CryptoPP::StringSink(hash_out),
                                       false
                                       )
                                   )
                              );
    return true;
}

static bool GenerateHexEncodedHmac(const std::string& source, std::string& hash_out) {
    CryptoPP::SHA512 hash;
    CryptoPP::StringSource src(source, 
                               true,
                               new CryptoPP::HashFilter( hash,
                                   new CryptoPP::HexEncoder(
                                       new CryptoPP::StringSink(hash_out),
                                       false
                                       )
                                   )
                              );
    return true;

}
static void GenerateFileHash(const std::string& filepath, std::string& hash_out) {
    CryptoPP::SHA512 hash;
    CryptoPP::FileSource src(filepath.c_str(),
                             true,
                             new CryptoPP::HashFilter( hash,
                                   new CryptoPP::Base64Encoder (
                                       new CryptoPP::StringSink(hash_out),
                                       false
                                       )
                                   )
                              );
} 

static int EncryptStringCFB(const std::string& data,
                            const Credentials& cred,
                            std::string& out) {
    int status = ret::A_OK;

    try {
        std::string cipher;

        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.byte_key(), cred.GetKeySize(), cred.byte_iv(), cred.GetIvSize());

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

static int DecryptStringCFB(const std::string& cipher,
                            const Credentials& cred,
                            std::string& out) {
    int status = ret::A_OK;

    try {
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption d;        
        d.SetKeyWithIV(cred.byte_key(), cred.GetKeySize(), cred.byte_iv(), cred.GetIvSize());

        CryptoPP::StringSource( cipher, 
                                true, 
                                new CryptoPP::StreamTransformationFilter( d,
                                    new CryptoPP::StringSink( out )
                                         ) // StreamTransformationFilter
                               ); // StringSource
    }
    catch (CryptoPP::Exception &e) {
        std::cout<<" EXCEPTION : " << e.what() << std::endl;
        status = ret::A_FAIL_DECRYPT;
    }                                                            

    return status;
}

static int EncryptStringGCM(const std::string& data,
                            const Credentials& cred,
                            std::string& out) {
    int status = ret::A_OK;
    try {
        CryptoPP::GCM<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(cred.byte_key(), cred.GetKeySize(), cred.byte_iv(), cred.GetIvSize());

        CryptoPP::StringSource( data,
                                true,
                                new CryptoPP::AuthenticatedEncryptionFilter( e,
                                new CryptoPP::StringSink(out),
                                false,
                                TAG_SIZE)
                              );
    }
    catch (CryptoPP::Exception &e) {
        std::cout<<" FAILED TO ENCRYPT GCM " << std::endl;
        status = ret::A_FAIL_ENCRYPT;
    }

    return status;
}

static int DecryptStringGCM(const std::string& cipher,
                            const Credentials& cred,
                            std::string& out) {
    try {
        CryptoPP::GCM<CryptoPP::AES>::Decryption d;        
        d.SetKeyWithIV(cred.byte_key(), cred.GetKeySize(), cred.byte_iv(), cred.GetIvSize());

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
        if (df.GetLastResult() == true ) {
            //std::cout<< "recovered text : " << rpdata << "\n";           
            // Write out data to ofstream
            out = rpdata;
        }                                                                

    }
    catch (CryptoPP::Exception &e) {
        std::cout << " Error : DecryptStringGCM : " << e.what() << std::endl;
        //std::cerr << e.what() << "\n";                           
        return ret::A_FAIL_DECRYPT;
    }                                                            
    return ret::A_OK;
}



static int GenerateKeyFromPassphrase(const std::string& pass, 
                                     const std::string& salt,
                                     Credentials& out) {
    int status = ret::A_OK;
    std::string outKey; //, outIv;

    // Check salt for size and correctness
    status = CheckSalt(salt);
    if(status == ret::A_OK) {
        ScryptEncode(pass, salt, outKey, CryptoPP::AES::MAX_KEYLENGTH); // MAX_KEYLENGTH for AES is 32
        //ScryptEncode(name, outIv, CryptoPP::AES::BLOCKSIZE);
        byte key[CryptoPP::AES::MAX_KEYLENGTH+1];
        memset(key, '\0', CryptoPP::AES::MAX_KEYLENGTH+1);
        memcpy(key, outKey.c_str(), CryptoPP::AES::MAX_KEYLENGTH);
        out.set_key(key, CryptoPP::AES::MAX_KEYLENGTH);

        byte iv[CryptoPP::AES::BLOCKSIZE+1];
        memset(iv, '\0', CryptoPP::AES::BLOCKSIZE+1);
        memcpy(iv, salt.c_str(), salt.size());
        out.set_iv(iv, salt.size());
    }

    return status;
}

static bool ScryptEncode(const std::string &input, 
                         const std::string &salt,
                         std::string &out,
                         unsigned int size) {
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

static int CheckSalt(const std::string& salt) {
    int status = ret::A_OK;

    // Check for correct size
    if(salt.size() != SALT_SIZE) {
        status = ret::A_FAIL_SCRYPT_INVALID_SALT_SIZE;
    }
    return status;
}

static int GenerateSalt(std::string& out) {
    int status = ret::A_OK;
    utils::GenerateRandomString(out, SALT_SIZE);
    return status;
}

static int GenerateHMACForString(const std::string& input,
                                 const Credentials& cred,
                                 std::string& macOut)
{
    int status = ret::A_OK;
    std::string mac;
    try {
        CryptoPP::HMAC<CryptoPP::SHA512> hmac(cred.byte_key(), cred.GetKeySize());
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
        CryptoPP::StringSource ss(mac,
                                  true,
                                  new CryptoPP::HexDecoder(
                                       new CryptoPP::StringSink(decoded)
                            ) // HexDecoder
        ); // StringSource

        if(!decoded.empty()) {
            CryptoPP::HMAC<CryptoPP::SHA512> hmac(cred.byte_key(), cred.GetKeySize());

            const int flags = CryptoPP::HashVerificationFilter::THROW_EXCEPTION | CryptoPP::HashVerificationFilter::HASH_AT_END;
        
            CryptoPP::StringSource(input + decoded, 
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


static void HexEncodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource(input,
                           true,
                           new CryptoPP::HexEncoder(
                             new CryptoPP::StringSink(output)
                           ) // HexEncoder
                          ); // StringSource
}

static void HexDecodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource(input,
                           true,
                           new CryptoPP::HexDecoder(
                               new CryptoPP::StringSink(output)
                           ) // HexDecoder
                          ); // StringSource
}

static void Base64EncodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource( input, 
                            true, 
                            new CryptoPP::Base64Encoder(new CryptoPP::StringSink(output), 
                                                        false) // Insert line breaks false
                          );
}

static void Base64DecodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource( input,
                            true,
                            new CryptoPP::Base64Decoder(new CryptoPP::StringSink(output)));
}

static void Base32EncodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource(input,
                           true,
                           new CryptoPP::Base32Encoder(new CryptoPP::StringSink(output))
                          );
}

static void Base32DecodeString(const std::string& input, std::string& output) {
    CryptoPP::StringSource(input,
                           true,
                           new CryptoPP::Base32Decoder(new CryptoPP::StringSink(output)));
}


static void GenerateRandomString(std::string& out, const unsigned int size = 16) {
    const unsigned int BLOCKSIZE = size * 8;
    byte pcbScratch[BLOCKSIZE];
    // Random Block
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock( pcbScratch, BLOCKSIZE );

    // Output
    std::string intermed;
    intermed.append(reinterpret_cast<const char*>(pcbScratch), BLOCKSIZE);
    Base64EncodeString(intermed, out);
}

}} //namespace
#endif

