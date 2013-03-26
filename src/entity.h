#ifndef ENTITY_H_
#define ENTITY_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "entityserver.h"

class AccessToken;
class Entity : public JsonSerializable {
public:
    typedef std::vector<EntityServer> ServerList;
    typedef std::vector<std::string> UrlList;

    Entity();
    ~Entity();

    int WriteToFile(const std::string& filepath);
    int LoadFromFile(const std::string& filepath);
    
    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    int Discover(const std::string& entityurl, const AccessToken* at);

    const std::string& entity() const           { return entity_; }
    const ServerList& server_list() const       { return server_list_; }
    const UrlList& previous_entities() const    { return previous_entities_; }
private:
    ServerList      server_list_;
    UrlList         previous_entities_;

    std::string     entity_;
};

#endif

