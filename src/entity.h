
#ifndef ENTITY_H_
#define ENTITY_H_
#pragma once

#include <vector>
#include <string>

#include "jsonserializable.h"
#include "profile.h"

class Entity : public JsonSerializable {
    void DeleteProfiles();
public:
    typedef std::vector<Profile> ProfileList;
    typedef std::vector<std::string> UrlList;
    
    Entity();
    Entity(const Entity& rhs);
    ~Entity();

    int WriteToFile(const std::string& filepath);
    int LoadFromFile(const std::string& filepath);
    
    void Serialize(Json::Value& root);
    void Deserialize(Json::Value& root);

    void PushBackProfile(Profile* pProf) { m_Profiles.push_back(*pProf); }
    void PushBackProfile(const Profile& prof) { m_Profiles.push_back(prof); }
    void PushBackProfileUrl(const std::string& url) { m_ProfileUrls.push_back(url); }

    const UrlList* GetProfileUrlList() const { return &m_ProfileUrls; }
    void GetFrontProfileUrl(std::string& out) { if(m_ProfileUrls.size()) out = m_ProfileUrls.front(); }
    ProfileList* GetProfileList(){ return &m_Profiles; }
    Profile* GetActiveProfile() const { return m_pActiveProfile; }
    Profile* GetFrontProfile() { if(m_Profiles.size()) { return &m_Profiles.front(); } return NULL; }

    AtticProfileInfo* GetAtticProfile() {
        AtticProfileInfo* pai = NULL;
        if(HasAtticProfile())
            pai = m_pActiveProfile->GetAtticInfo();
        return pai;
    }

    void GetEntityUrl(std::string& out) const { out = m_EntityUrl; }
    void GetApiRoot(std::string& out) const { out = m_ApiRoot; }
    unsigned int GetProfileCount() { return m_ProfileUrls.size(); }
    
    void SetEntityUrl(const std::string& url)       { m_EntityUrl = url; }
    void SetApiRoot(const std::string& root)        { m_ApiRoot = root; }
    void SetActiveProfile(Profile* pProf)           { m_pActiveProfile = pProf; }

    bool HasAtticProfile() { if(m_pActiveProfile && m_pActiveProfile->GetAtticInfo()) return true ; return false; }

    bool HasAtticProfileMasterKey();

    void Reset();

private:
    ProfileList     m_Profiles;
    UrlList         m_ProfileUrls;
    std::string     m_EntityUrl;
    std::string     m_ApiRoot;

    Profile*        m_pActiveProfile;
};

#endif

