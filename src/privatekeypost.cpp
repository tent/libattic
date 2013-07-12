#include "privatekeypost.h"
#include "constants.h"

namespace attic { 

PrivateKeyPost::PrivateKeyPost() {
    set_type(cnst::g_attic_privatekey_type);
}

PrivateKeyPost::~PrivateKeyPost() {}

void PrivateKeyPost::Serialize(Json::Value& root) {
    Json::Value keys(Json::objectValue);
    keys["public_key"] = public_key_;
    keys["private_key"] = private_key_;
    set_content("keys", keys);
    Post::Serialize(root);
}

void PrivateKeyPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value keys;
    get_content("keys", keys);
    public_key_ = keys.get("public_key", "").asString();
    private_key_ = keys.get("private_key", "").asString();
}

}//namespace


