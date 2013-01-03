
#ifndef ENTITY_H_
#define ENTITY_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "profile.h"

class Entity : public JsonSerializable
{
public:
    typedef std::vector<Profile*> ProfileList;
    typedef std::vector<std::string> UrlList;
    
    Entity();
    ~Entity();

    int WriteToFile(const std::string& filepath);
    int LoadFromFile(const std::string& filepath);
    
    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void PushBackProfile(Profile* pProf) { m_Profiles.push_back(pProf); }
    void PushBackProfileUrl(const std::string& url) { m_ProfileUrls.push_back(url); }

    const UrlList* GetProfileList() const { return &m_ProfileUrls; }
    void GetFrontProfileUrl(std::string& out) { if(m_ProfileUrls.size()) out = m_ProfileUrls.front(); }
    void GetEntityUrl(std::string& out) const { out = m_EntityUrl; }
    void GetApiRoot(std::string& out) const { out = m_ApiRoot; }
    unsigned int GetProfileCount() { return m_ProfileUrls.size(); }
    
    void SetEntityUrl(const std::string& url) { m_EntityUrl = url; }
    void SetApiRoot(const std::string& root) { m_ApiRoot = root; }

private:
    ProfileList     m_Profiles;
    UrlList         m_ProfileUrls;
    std::string     m_EntityUrl;
    std::string     m_ApiRoot;
};

#endif

