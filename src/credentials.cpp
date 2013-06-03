#include "credentials.h"

#include <string.h>
#include "errorcodes.h"

#include "crypto.h"

namespace attic {

Credentials::Credentials() {
    memset(byte_key_, '\0', CryptoPP::AES::MAX_KEYLENGTH+1);
    memset(byte_iv_, '\0', CryptoPP::AES::BLOCKSIZE+1);
}

Credentials::~Credentials() {}

void Credentials::Serialize(Json::Value& root) {
    root["key"] = key_;
    root["iv"] = iv_;
}

void Credentials::Deserialize(Json::Value& root) {
    std::string key, iv;
    key = root.get("key", "").asString();
    iv = root.get("iv", "").asString();

    // Do not set directly
    set_key(key); 
    set_iv(iv);
}

void Credentials::GetSerializedCredentials(std::string& out) {
    jsn::SerializeObject(this, out);
}

std::string Credentials::asString() const {
    Credentials obj = *this;
    std::string out;
    jsn::SerializeObject(&obj, out);
    return out;
}

int Credentials::set_key(const std::string& key) { 
    int status = ret::A_OK;
    status = set_key(reinterpret_cast<const byte*>(key.c_str()), GetKeySize());
    return status;
}

int Credentials::set_key(const byte* pKey, const unsigned int length) {
    int status = ret::A_OK;

    if(length <= GetKeySize()) {
        memset(byte_key_, '\0', GetKeySize()+1);
        memcpy(byte_key_, pKey, length);
        key_.clear();
        key_.append(reinterpret_cast<const char*>(byte_key_), length);
    }
    else {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}

int Credentials::set_iv(const std::string& iv) { 
    int status = ret::A_OK;
    status = set_iv(reinterpret_cast<const byte*>(iv.c_str()), iv.size());
    return status;
}

int Credentials::set_iv(const byte* pIv, const unsigned int length) {
    int status = ret::A_OK;
    if(length <= GetIvSize()) {
        memset(byte_iv_, '\0', CryptoPP::AES::BLOCKSIZE+1);
        memcpy(byte_iv_, pIv, length);
        iv_.clear();
        iv_.append(reinterpret_cast<const char*>(byte_iv_), length);
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
