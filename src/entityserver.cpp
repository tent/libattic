#include "entityserver.h"

namespace attic { 

void ServerUrls::Serialize(Json::Value& root) {
    root["app_auth_request"] = app_auth_request;
    root["app_token_request"] = app_token_request;
    root["posts_feed"] = posts_feed;
    root["new_post"] = new_post;
    root["post"] = post;
    root["post_attachment"] = post_attachment;
    root["batch"] = batch;
    root["server_info"] = server_info;
}

void ServerUrls::Deserialize(Json::Value& root) {
    app_auth_request  = root.get("app_auth_request", "").asString();
    app_token_request = root.get("app_token_request", "").asString();
    posts_feed = root.get("posts_feed", "").asString();
    new_post = root.get("new_post", "").asString();
    post = root.get("post", "").asString();
    post_attachment = root.get("post_attachment", "").asString();
    batch = root.get("batch", "").asString();
    server_info = root.get("server_info", "").asString();
}

EntityServer::EntityServer() {}
EntityServer::~EntityServer() {}

void EntityServer::Serialize(Json::Value& root) {
    root["version"] = version_;
    root["preference"] = preference_;

    Json::Value urlobj(Json::objectValue);
    urls_.Serialize(urlobj);
    root["urls"] = urlobj;
}

void EntityServer::Deserialize(Json::Value& root) {
    version_ = root.get("version", "").asString();
    preference_ = root.get("preference", "").asString();

    urls_.Deserialize(root["urls"]);
}

}//namespace
