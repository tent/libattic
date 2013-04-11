#ifndef ENTITYSERVER_H_
#define ENTITYSERVER_H_
#pragma once

#include <string>
#include "jsonserializable.h"

namespace attic { 

struct ServerUrls : public JsonSerializable {
    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    std::string oauth_auth;
    std::string oauth_token;
    std::string posts_feed;
    std::string new_post;
    std::string post;
    std::string post_attachment; // Depricated
    std::string attachment; // use this instead
    std::string batch;
    std::string server_info;
};

class EntityServer : public JsonSerializable { 
public:
    EntityServer();
    ~EntityServer();

    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    const std::string& version() const              { return version_; }
    const std::string& preference() const           { return preference_; }
    const std::string& oauth_auth() const           { return urls_.oauth_auth; }
    const std::string& oauth_token() const          { return urls_.oauth_token; }
    const std::string& attachment() const           { return urls_.attachment; }
    const std::string& posts_feed() const           { return urls_.posts_feed; }
    const std::string& new_post() const             { return urls_.new_post; }
    const std::string& post() const                 { return urls_.post; }
    const std::string& post_attachment() const      { return urls_.post_attachment; }
    const std::string& batch() const                { return urls_.batch; } 
    const std::string& server_info() const          { return urls_.server_info; }
private:
    ServerUrls  urls_;
    std::string version_;
    std::string preference_;
};

} //namespace
#endif

