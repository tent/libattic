#include "apppost.h"

void AppPost::Serialize(Json::Value& root) {
    Json::Value name, url, redirect_uri;
    name = name_;
    url = url_;
    redirect_uri = redirect_uri_;
    set_content("name", name);
    set_content("url", url);
    set_content("redirect_uri", redirect_uri);

    Json::Value posttype(Json::objectValue);
    Json::Value read_types(Json::arrayValue);
    Json::Value write_types(Json::arrayValue);
    jsn::SerializeVector(post_types_["read"], read_types);
    jsn::SerializeVector(post_types_["write"], write_types);
    posttype["read"] = read_types;
    posttype["write"] = write_types;
    set_content("post_types", posttype);

    Post::Serialize(root);
}

void AppPost::Deserialize(Json::Value& root) {
    Post::Deserialize(root);
    Json::Value name, url, redirect_uri;
    get_content("name", name);
    get_content("url", url);
    get_content("redirect_uri", redirect_uri);
    name_ = name.asString();
    url_ = url.asString();
    redirect_uri_ = redirect_uri.asString();

    Json::Value posttype(Json::objectValue);
    get_content("post_types", posttype);

    jsn::DeserializeIntoVector(posttype["read"], post_types_["read"]);
    jsn::DeserializeIntoVector(posttype["write"], post_types_["write"]);
}

void AppPost::PushBackWriteType(const std::string& type) {
    post_types_["write"].push_back(type);
}

void AppPost::PushBackReadType(const std::string& type) {
    post_types_["read"].push_back(type);
}

