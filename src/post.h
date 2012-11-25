


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
        std::cout<< " HERE " << std::endl;
        Type = root.get("type", "").asString();
        
        std::cout<< " HERE " << std::endl;
        Category = root.get("category", "").asString();
        std::cout<< " HERE " << std::endl;
        Name = root.get("name", "").asString();
        std::cout<< " HERE " << std::endl;
        Size = root.get("size", 0).asUInt();
        std::cout<< " HERE " << std::endl;
    }

    
};

class Post : public JsonSerializable
{
    typedef std::vector<Attachment*> AttachmentVec;

public:
    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    std::string GetID() { return m_ID; }

    void SetID(const std::string &szId) { m_ID = szId; }
    void setEntity(const std::string &szEntity) { m_Entity = szEntity; }
    void SetPublishedAt(unsigned int uUnixTime) { m_PublishedAt = uUnixTime; }

    void SetType(const std::string &szType) { m_Type = szType; }

    void SetContent(const std::string &szType, const std::string &szVal) { m_Content[szType] = szVal; }

    void PushBackAttachment(Attachment* pAtch) { m_Attachments.push_back(pAtch); }
    
    void SetPermission(const std::string &szPermission, bool bVal) { m_Permissions[szPermission] = bVal; }

    AttachmentVec* GetAttachments() { return &m_Attachments; }
    unsigned int GetVersion() { return m_Version; }

private:
    std::string                         m_ID;
    std::string                         m_Entity;
    unsigned int                        m_PublishedAt;
    unsigned int                        m_ReceivedAt;
    std::vector<std::string>            m_Mentions;
    std::vector<std::string>            m_Licenses;
    std::string                         m_Type;
    std::map<std::string, std::string>  m_Content;

    AttachmentVec                       m_Attachments;

    TentApp*                            m_TentApp;
    std::map<std::string, std::string>  m_Views;
    std::map<std::string, bool>         m_Permissions;

    unsigned int                        m_Version;
};

#endif


