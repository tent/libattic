#ifndef CRYPTO_H_
#define CRYPTO_H_
#pragma once

#include <string>
#include <fstream>

// Depricated
#include <hex.h>         
#include <filters.h>     
#include <gcm.h>         
#include <hmac.h>
#include <aes.h>
#include <osrng.h>
#include <base32.h>
#include <base64.h>
#include <files.h>
// Depricated //

#include <sodium.h>

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
// new
static bool ScryptEncode(const std::string &input, 
                         const std::string &salt,
                         const unsigned int size,
                         std::string &out);

static void GenerateKey(std::string& out);
static void GenerateIv(std::string& out);
static void GenerateNonce(std::string& out);
static bool Encrypt(const std::string& in, const Credentials& cred, std::string& out);
static bool Decrypt(const std::string& in, const Credentials& cred, std::string& out);
static int EnterPassphrase(const std::string& pass, const std::string& iv, Credentials& out);
static int GenerateKeyFromPassphrase(const std::string& pass, Credentials& out);
static Credentials GenerateCredentials();
static void GenerateCredentials(Credentials& cred);

// Old
static const int TAG_SIZE = 16;
static const int SALT_SIZE = 16;
static CryptoPP::AutoSeededRandomPool  g_Rnd;                // Random pool used for key generation
static unsigned int                    g_Stride = 400000;    // Size of stride used when encrypting


static void GenerateIv(std::string& out);
static bool GenerateHash( const std::string& source, std::string& hash_out);
static bool GenerateHexEncodedHmac(const std::string& source, std::string& hash_out);
static void GenerateFileHash(const std::string& filepath, std::string& hash_out);
static void GenerateRandomString(std::string& out, const unsigned int size);

static int GenerateKeyFromPassphrase( const std::string& pass, 
                                      const std::string& salt,
                                      Credentials& out);

static int GenerateHMACForString(const std::string& input,
                                 const Credentials& cred,
                                 std::string& macOut);

static int VerifyHMACForString(const std::string& input,
                               const Credentials& cred,
                               const std::string& mac);



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
    CryptoPP::StringSource(input, 
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


// New NACL STUFF
//
//
//
static Credentials GenerateCredentials() {
    Credentials cred;
    GenerateCredentials(cred);
    return cred;
}

static void GenerateCredentials(Credentials& cred) {
    std::string key, iv;
    GenerateIv(iv);
    GenerateKey(key);
    cred.set_key(key);
    cred.set_iv(iv);
}

static bool Encrypt(const std::string& in, const Credentials& cred, std::string& out) {
    std::string buffer;
    // append buffer, (required by nacl)
    buffer.append(32,0);
    // append data
    buffer.append(in.c_str(), in.size());
    // ciphertext
    unsigned char c[buffer.size()];
    if(crypto_secretbox(c,  
                        reinterpret_cast<const unsigned char*>(buffer.c_str()),
                        buffer.size(),
                        cred.byte_iv(),
                        cred.byte_key()) == 0) {
        out.append(reinterpret_cast<const char*>(c), buffer.size());
        return true;
    }
    return false;
}

static bool Decrypt(const std::string& in, const Credentials& cred, std::string& out) {
    unsigned char m[in.size()];
    if(crypto_secretbox_open(m, 
                             reinterpret_cast<const unsigned char*>(in.c_str()),
                             in.size(),
                             cred.byte_iv(),
                             cred.byte_key()) == 0) {
        out.append(reinterpret_cast<const char*>(m), in.size());
        // remove padding
        out.erase(0, 32);
        return true;
    }
    return false;
}

static void GenerateKey(std::string& out) {
    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes(key, crypto_secretbox_KEYBYTES);
    out.append(reinterpret_cast<const char*>(key), crypto_secretbox_KEYBYTES);
}

static void GenerateNonce(std::string& out) {
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes(nonce, crypto_secretbox_NONCEBYTES);
    out.append(reinterpret_cast<const char*>(nonce), crypto_secretbox_NONCEBYTES);
}

static void GenerateIv(std::string& out) {
    GenerateNonce(out);
}

static int EnterPassphrase(const std::string& pass, const std::string& iv, Credentials& out) {
    int status = ret::A_OK;
    std::string key;
    ScryptEncode(pass, iv, out.GetKeySize(), key);
    // Set key
    status = out.set_key(key);
    if(status == ret::A_OK) {
        // Set iv
        status = out.set_iv(iv);
    }
    return status;
}

static int GenerateKeyFromPassphrase(const std::string& pass, Credentials& out) {
    int status = ret::A_OK;
    std::string key, salt;
    GenerateNonce(salt);
    status = EnterPassphrase(pass, salt, out);
    return status;
}

static bool ScryptEncode(const std::string &input, 
                         const std::string &salt,
                         const unsigned int size,
                         std::string &out) {
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
    out.append(reinterpret_cast<char*>(dk), sizeof(uint8_t)*size);

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

}} //namespace
#endif

