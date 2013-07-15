#include "sharedfilepost.h"

#include "crypto.h"
#include "constants.h"

namespace attic { 

SharedFilePost::SharedFilePost() {
    set_type(cnst::g_attic_shared_file_type);
}

SharedFilePost::~SharedFilePost() {}

void SharedFilePost::Serialize(Json::Value& root) {
    Json::Value shared_cred(Json::objectValue);

    std::string key_data;
    crypto::Base64EncodeString(encrypted_key_, key_data);
    shared_cred["key"] = key_data;

    std::string iv_data;
    crypto::Base64EncodeString(iv_, iv_data);
    shared_cred["iv"] = iv_data;

    set_content("shared_credentials", shared_cred);
    Post::Serialize(root);
}

void SharedFilePost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);

    Json::Value shared_cred(Json::objectValue);
    get_content("shared_credentials", shared_cred);

    std::string key_data;
    crypto::Base64DecodeString(shared_cred["key"].asString(), key_data);
    encrypted_key_ = key_data;

    std::string iv_data;
    crypto::Base64DecodeString(shared_cred["iv"].asString(), iv_data);
    iv_ = iv_data;
}

void SharedFilePost::set_encrypted_key(const std::string& key) { 
    // avoid cow
    encrypted_key_.append(key.c_str(), key.size()); 
}

void SharedFilePost::set_iv(const std::string& iv) { 
    // avoid cow
    iv_.append(iv.c_str(), iv.size());
}


} //namespace
