#ifndef SHAREDFILEPOST_H_
#define SHAREDFILEPOST_H_
#pragma once

#include <string>
#include "post.h"

namespace attic {

class SharedFilePost : public Post { 
public:
    SharedFilePost();
    ~SharedFilePost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const std::string& encrypted_key() const { return encrypted_key_; }
    const std::string& iv() const { return iv_; }

    void set_encrypted_key(const std::string& key);
    void set_iv(const std::string& iv);
private:
    std::string encrypted_key_;
    std::string iv_;
};


}//namespace 
#endif

