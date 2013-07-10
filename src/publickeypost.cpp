#include "publickeypost.h"

#include "constants.h"

namespace attic { 

PublicKeyPost::PublicKeyPost() {
    set_type(cnst::g_attic_publickey_type);
    set_public(true);
}

PublicKeyPost::~PublicKeyPost() {}

void PublicKeyPost::Serialize(Json::Value& root) {
    Json::Value key(Json::objectValue);
    key["key"] = public_key_;
    set_content("public_key", key);
    Post::Serialize(root);
}

void PublicKeyPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value key;
    get_content("public_key", key);
    public_key_ = key.get("key", "").asString();
}

} //namespace

