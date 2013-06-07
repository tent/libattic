#include "credentials.h"

#include <string.h>
#include "errorcodes.h"

#include "crypto.h"

namespace attic {

Credentials::Credentials() {
    memset(byte_key_, 0, crypto_secretbox_KEYBYTES);
    memset(byte_iv_, 0, crypto_secretbox_NONCEBYTES);
}

Credentials::~Credentials() {}

int Credentials::set_key(const std::string& key) { 
    int status = ret::A_OK;
    status = set_key(reinterpret_cast<const unsigned char*>(key.c_str()), key.size());
    return status;
}

int Credentials::set_key(const unsigned char* key, const unsigned int len) {
    int status = ret::A_OK;
    if(len <= GetKeySize()) {
        memset(byte_key_, 0, GetKeySize());
        memcpy(byte_key_, key, len);
        key_.clear();
        key_.append(reinterpret_cast<const char*>(byte_key_), len);
    }
    else {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }
    return status;
}

int Credentials::set_iv(const std::string& iv) { 
    int status = ret::A_OK;
    status = set_iv(reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    return status;
}

int Credentials::set_iv(const unsigned char* iv, const unsigned int len) {
    int status = ret::A_OK;
    if(len <= GetIvSize()) {
        memset(byte_iv_, 0, crypto_secretbox_NONCEBYTES);
        memcpy(byte_iv_, iv, len);
        iv_.clear();
        iv_.append(reinterpret_cast<const char*>(byte_iv_), len);
    }
    else {
        status = ret::A_FAIL_IVSIZE_MISMATCH;
    }
    return status;
}

bool Credentials::byte_key_empty() {
    if(strlen(reinterpret_cast<const char*>(byte_key_)))
        return false;
    return true;
}

bool Credentials::byte_iv_empty() {
    if(strlen(reinterpret_cast<const char*>(byte_iv_)))
        return false;
    return true;
}


}//namespace
