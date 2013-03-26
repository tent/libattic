#ifndef POST_H_
#define POST_H_
#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "jsonserializable.h"
#include "permissions.h"

class TentApp;


struct Version : public JsonSerializable {
    std::string id;
    std::string type;
    std::string published_at;
    std::string received_at;

    void Serialize(Json::Value& root) {
        root["id"] = id;
        root["type"] = type;
    }

    void Deserialize(Json::Value& root) {
        id = root.get("id", "").asString();
        type = root.get("type", "").asString();
        published_at = root.get("published_at", "").asString();
        received_at = root.get("received_at", "").asString();
    }
};

struct Attachment : public JsonSerializable {
    std::string Type;      // `json:"type"`
    std::string Category;  // `json:"category"`
    std::string Name;      // `json:"name"`
    unsigned int Size;     // `json:"size"`

    void AssignKeyValue(const std::string &key, const Json::Value &val) {
        if(key == std::string("type")) {
            Type = val.asString();
            return;
        }
        if(key == std::string("category")) {
            Category = val.asString();
            return;
        }
        if(key == std::string("name")) {
            Name = val.asString();
            return;
        }
        if(key == std::string("size")) {
            Size = val.asUInt();
            return;
        }
        
        std::cout<< "Uknown key : " << key << std::endl;
    }

    virtual void Serialize(Json::Value& root) {
        // TODO :: this, later
        std::cout<<" attatchment serialize UNIMPLEMENTED " << std::endl;
    }

    virtual void Deserialize(Json::Value& root) {
        Type = root.get("type", "").asString();
        Category = root.get("category", "").asString();
        Name = root.get("name", "").asString();
        Size = root.get("size", 0).asUInt();
    }
    
};

class Post : public JsonSerializable {
public:
    typedef std::vector<Attachment> AttachmentVec;

    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void GetID(std::string& out) const          { out = m_ID; }
    void GetPostType(std::string& out) const    { out = m_Type; }
    void GetContent(const std::string& key, Json::Value& out);
    unsigned int GetAttachmentCount()           { return m_Attachments.size(); }
    AttachmentVec* GetAttachments()             { return &m_Attachments; }

    void SetID(const std::string &id)           { m_ID = id; }
    void setEntity(const std::string &entity)   { m_Entity = entity; }
    void SetPublishedAt(unsigned int uUnixTime) { m_PublishedAt = uUnixTime; }
    void SetPostType(const std::string &type)   { m_Type = type; }

    void SetContent(const std::string &type, Json::Value &val) { m_Content[type] = val; }

    void SetPublic(const bool pub) { m_Permissions.SetIsPublic(pub); }

    void PushBackAttachment(Attachment& pAtch) { m_Attachments.push_back(pAtch); }

private:
    std::string                         m_ID;
    std::string                         m_Entity;
    unsigned int                        m_PublishedAt;
    unsigned int                        m_ReceivedAt;
    std::vector<std::string>            m_Mentions;
    std::vector<std::string>            m_Licenses;
    std::string                         m_Type;

    //typedef std::map<std::string, std::string> ContentMap;
    typedef std::map<std::string, Json::Value> ContentMap;
    ContentMap                          m_Content;

    AttachmentVec                       m_Attachments;

    TentApp*                            m_TentApp;
    std::map<std::string, std::string>  m_Views;
    //std::map<std::string, bool>         m_Permissions;
    Permissions m_Permissions;

    Version version_;


};

#endif

