#ifndef PRIVATEKEYPOST_H_
#define PRIVATEKEYPOST_H_
#pragma once

#include <string>
#include "post.h"

namespace attic {

class PrivateKeyPost : public Post {
public:
    PrivateKeyPost();
    ~PrivateKeyPost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);


    const std::string& public_key() const { return public_key_; }
    const std::string& private_key() const { return private_key_; }

    void set_public_key(const std::string& key) { public_key_ = key; }
    void set_private_key(const std::string& key) { private_key_ = key; }

private:
    std::string private_key_;
    std::string public_key_;
};

}//namespace
#endif

