


#ifndef POST_H_
#define POST_H_
#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "jsonserializable.h"

class TentApp;

struct Attachment : public JsonSerializable
{
    std::string Type;      // `json:"type"`
    std::string Category;  // `json:"category"`
    std::string Name;      // `json:"name"`
    unsigned int Size;     // `json:"size"`
    char* pData;

    void AssignKeyValue(const std::string &key, const Json::Value &val)
    {
        if(key == std::string("type"))
        {
            Type = val.asString();
            return;
        }
        if(key == std::string("category"))
        {
            Category = val.asString();
            return;
        }
        if(key == std::string("name"))
        {
            Name = val.asString();
            return;
        }
        if(key == std::string("size"))
        {
            Size = val.asUInt();
            return;
        }
        
        std::cout<< "Uknown key : " << key << std::endl;
    }

    virtual void Serialize(Json::Value& root)
    {
        // TODO :: this, later
    }

    virtual void Deserialize(Json::Value& root)
    {
        Type = root.get("type", "").asString();
        Category = root.get("category", "").asString();
        Name = root.get("name", "").asString();
        Size = root.get("size", 0).asUInt();
    }
    
};

class Post : public JsonSerializable
{


public:
    typedef std::vector<Attachment*> AttachmentVec;

    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void GetID(std::string& out)        { out = m_ID; }
    void GetPostType(std::string& out)  { out = m_Type; }
    void GetContent(const std::string& key, std::string& out);
    unsigned int GetAttachmentCount()   { return m_Attachments.size(); }
    AttachmentVec* GetAttachments()     { return &m_Attachments; }
    unsigned int GetVersion()           { return m_Version; }

    void SetID(const std::string &id)           { m_ID = id; }
    void setEntity(const std::string &entity)   { m_Entity = entity; }
    void SetPublishedAt(unsigned int uUnixTime) { m_PublishedAt = uUnixTime; }
    void SetPostType(const std::string &type)   { m_Type = type; }
    void SetContent(const std::string &type, const std::string &val) { m_Content[type] = val; }
    void SetPermission(const std::string &permission, bool Val) { m_Permissions[permission] = Val; }

    void PushBackAttachment(Attachment* pAtch) { m_Attachments.push_back(pAtch); }

private:
    std::string                         m_ID;
    std::string                         m_Entity;
    unsigned int                        m_PublishedAt;
    unsigned int                        m_ReceivedAt;
    std::vector<std::string>            m_Mentions;
    std::vector<std::string>            m_Licenses;
    std::string                         m_Type;

    typedef std::map<std::string, std::string> ContentMap;
    ContentMap                          m_Content;

    AttachmentVec                       m_Attachments;

    TentApp*                            m_TentApp;
    std::map<std::string, std::string>  m_Views;
    std::map<std::string, bool>         m_Permissions;

    unsigned int                        m_Version;
};

#endif


