#include "entitypost.h"

EntityPost::EntityPost() {}
EntityPost::~EntityPost() {}

void EntityPost::Serialize(Json::Value& root) {
    Json::Value entityval;
    jsn::SerializeObject(&entity_, entityval);

    Json::Value server_arr(Json::arrayValue);
    entity_.SerializeServerList(server_arr);

    Json::Value prev_ent(Json::arrayValue);
    entity_.SerializePreviousEntities(prev_ent);

    Json::Value entity_url = entity().entity();
    SetContent("entity", entity_url);
    SetContent("previous_entities", prev_ent);
    SetContent("servers", server_arr);

    Post::Serialize(root);
}

void EntityPost::Deserialize(Json::Value& root) {

    Post::Deserialize(root);

    Json::Value server_arr(Json::arrayValue);
    Json::Value prev_ent(Json::arrayValue);
    GetContent("servers", server_arr);
    GetContent("previous_entities", prev_ent);
    Json::Value entity_url;
    GetContent("entity", entity_url);

    entity_.set_entity(entity_url.asString());
    entity_.DeserializeServerList(server_arr);
    entity_.DeserializePreviousEntities(prev_ent);
}

