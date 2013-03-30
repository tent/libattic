
#ifndef PERMISSIONS_H_
#define PERMISSIONS_H_
#pragma once

#include <string>
#include <vector>

#include "jsonserializable.h"

namespace attic { 

class Group : public JsonSerializable
{
public:
    Group()
    {
        m_CreatedAt = 0;
    }
    ~Group() {}
    
     void Serialize(Json::Value& root)
    {
        root["id"] = m_Id;
        root["created_at"] = m_CreatedAt;
        root["name"] = m_Name;
    }

    void Deserialize(Json::Value& root)
    {
        m_Id = root.get("id", "").asString();
        m_CreatedAt = root.get("created_at", "").asInt();
        m_Name = root.get("name", "").asString();
    }

private:
    std::string m_Id;
    std::string m_Name;
    int m_CreatedAt;

};


class Permissions : public JsonSerializable
{
public:
    typedef std::map<std::string, bool> EntityMap;
    typedef std::vector<Group> GroupList;

    Permissions()
    {
        m_Public = false;
    }

    ~Permissions() { } 

    void Serialize(Json::Value& root)
    {
        root["public"] = m_Public;

        Json::Value groups;
        for(unsigned int i=0; i< m_Groups.size(); i++)
            jsn::SerializeObject(&m_Groups[i], groups );

        root["groups"] = groups;

        Json::Value entities(Json::arrayValue);
        jsn::SerializeMapIntoObject(entities, m_EntityPermissionMap);

        root["entities"] = entities;
    }

    void Deserialize(Json::Value& root)
    {
        m_Public = root.get("public", false).asBool();

        if(root["groups"].isObject())
        {
            Json::ValueIterator itr = root["groups"].begin();

            for(; itr != root["groups"].end(); itr++)
            {
                Group g;
                jsn::DeserializeObject(&g, *itr);
                m_Groups.push_back(g);
            }

        }

        jsn::DeserializeObjectValueIntoMap(root["entities"], m_EntityPermissionMap);
    }

    bool GetIsPublic() const { return m_Public; }
    void SetIsPublic(const bool pub) { m_Public = pub; }

private:
    EntityMap m_EntityPermissionMap;
    GroupList m_Groups;
    bool    m_Public;
};

}//namespace
#endif

