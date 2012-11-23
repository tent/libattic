


#ifndef POST_H_
#define POST_H_
#pragma once

#include <string>
#include <vector>
#include <map>

#include "jsonserializable.h"

class TentApp;

struct Attachment
{
    std::string Type;      // `json:"type"`
    std::string Category;  // `json:"category"`
    std::string Name;      // `json:"name"`
    unsigned int Size;     // `json:"size"`
    char* pData;
};

class Post : public JsonSerializable
{

public:
    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

    void SetID(const std::string &szId) { m_ID = szId; }
    void setEntity(const std::string &szEntity) { m_Entity = szEntity; }
    void SetPublishedAt(unsigned int uUnixTime) { m_PublishedAt = uUnixTime; }

    void SetType(const std::string &szType) { m_Type = szType; }

    void SetContent(const std::string &szType, const std::string &szVal)
    {
        m_Content[szType] = szVal;
    }

    void SetAttachment(const std::string &szAttachment) { m_Attachments.push_back(szAttachment); }
    
    void SetPermission(const std::string &szPermission, bool bVal)
    {
        m_Permissions[szPermission] = bVal;
    }

private:
    std::string                         m_ID;
    std::string                         m_Entity;
    unsigned int                        m_PublishedAt;
    unsigned int                        m_ReceivedAt;
    std::vector<std::string>            m_Mentions;
    std::vector<std::string>            m_Licenses;
    std::string                         m_Type;
    std::map<std::string, std::string>  m_Content;
    std::vector<std::string>            m_Attachments;
    TentApp*                            m_TentApp;
    std::map<std::string, std::string>  m_Views;
    std::map<std::string, bool>         m_Permissions;
};

#endif


