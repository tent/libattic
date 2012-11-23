


#ifndef POST_H_
#define POST_H_
#pragma once

#include <string>
#include <vector>
#include <map>

#include "jsonserializable.h"

class TentApp;

class Post : public JsonSerializable
{

public:
    Post();
    ~Post();

    virtual void Serialize(Json::Value& root);
    virtual void Deserialize(Json::Value& root);

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


