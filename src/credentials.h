#ifndef CREDENTIALS_H_
#define CREDENTIALS_H_
#pragma once

#include <sodium.h>
#include <string>
#include <stdio.h>

namespace attic { 

class Credentials  {
public:
    Credentials();
    ~Credentials();

    unsigned int GetKeySize() const { return sizeof(unsigned char) * crypto_secretbox_KEYBYTES; }
    unsigned int GetIvSize() const { return sizeof(unsigned char) * crypto_secretbox_NONCEBYTES; } 

    const std::string& key() const { return key_; }
    const std::string& iv() const { return iv_; }

    const unsigned char* byte_key() const { return byte_key_; }
    const unsigned char* byte_iv() const { return byte_iv_; }

    int set_key(const std::string& key);
    int set_key(const unsigned char* key, const unsigned int len);

    int set_iv(const std::string& iv); 
    int set_iv(const unsigned char* iv, const unsigned int len);

    bool byte_key_empty();
    bool byte_iv_empty();

    bool key_empty() const { return key_.empty(); }
    bool iv_empty() const { return iv_.empty(); } 

private:
    std::string key_;
    std::string iv_;

    unsigned char byte_key_[crypto_secretbox_KEYBYTES];
    unsigned char byte_iv_[crypto_secretbox_NONCEBYTES]; 
};

} //namespace
#endif

