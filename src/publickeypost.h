#ifndef PUBLICKEYPOST_H_
#define PUBLICKEYPOST_H_
#pragma once

#include <string>
#include "post.h"

namespace attic {

class PublicKeyPost : public Post {
public:
    PublicKeyPost();
    ~PublicKeyPost();

    void Serialize(Json::Value& root);  
    void Deserialize(Json::Value& root);

    const std::string& public_key() const { return public_key_; }
    void set_public_key(const std::string& key) { public_key_ = key; }
private:
    std::string public_key_;
};

}// namespace

#endif

