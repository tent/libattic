#ifndef PERMISSIONS_H_
#define PERMISSIONS_H_
#pragma once

#include <string>
#include <vector>

#include "jsonserializable.h"

namespace attic { 

class Group : public JsonSerializable {
public:
    Group() {
        create_at_ = 0;
    }
    ~Group() {}
    
    void Serialize(Json::Value& root) {
        root["id"] = id_;
        root["created_at"] = create_at_;
        root["name"] = name_;
    }

    void Deserialize(Json::Value& root) {
        id_ = root.get("id", "").asString();
        create_at_ = root.get("created_at", "").asInt();
        name_ = root.get("name", "").asString();
    }

private:
    std::string id_;
    std::string name_;
    int create_at_;
};

class Permissions : public JsonSerializable {
public:
    typedef std::map<std::string, bool> EntityMap;
    typedef std::vector<Group> GroupList;

    Permissions() {
        public_ = false;
    }

    ~Permissions() { } 

    void Serialize(Json::Value& root) {
        root["public"] = public_;
        Json::Value groups;
        for(unsigned int i=0; i< groups_.size(); i++)
            jsn::SerializeObject(&groups_[i], groups );
        root["groups"] = groups;
        Json::Value entities(Json::arrayValue);
        jsn::SerializeMapIntoObject(entities, entity_map_);
        root["entities"] = entities;
    }

    void Deserialize(Json::Value& root) {
        public_ = root.get("public", false).asBool();
        if(root["groups"].isObject()) {
            Json::ValueIterator itr = root["groups"].begin();
            for(; itr != root["groups"].end(); itr++) {
                Group g;
                jsn::DeserializeObject(&g, *itr);
                groups_.push_back(g);
            }
        }
        jsn::DeserializeObjectValueIntoMap(root["entities"], entity_map_);
    }

    bool GetIsPublic() const { return public_; }
    void SetIsPublic(const bool pub) { public_ = pub; }
private:
    EntityMap entity_map_;
    GroupList groups_;
    bool    public_;
};

}//namespace
#endif

