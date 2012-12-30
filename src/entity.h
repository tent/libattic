
#ifndef ENTITY_H_
#define ENTITY_H_
#pragma once

#include <vector>
#include <string>

#include "mutexclass.h"

class Entity
{
public:
    Entity(){}
    ~Entity(){}

    typedef std::vector<std::string> ProfileList;
    void PushBackProfileUrl(const std::string& url) { m_ProfileUrls.push_back(url); }
    const ProfileList* GetProfileList() const { return &m_ProfileUrls; }
    

    void GetEntityUrl(std::string& out) const { out = m_EntityUrl; }
    void GetApiRoot(std::string& out) const { out = m_ApiRoot; }
    unsigned int GetProfileCount() { return m_ProfileUrls.size(); }
    
    void SetEntityUrl(const std::string& url) { m_EntityUrl = url; }
    void SetApiRoot(const std::string& root) { m_ApiRoot = root; }

private:
    ProfileList     m_ProfileUrls;
    std::string     m_EntityUrl;
    std::string     m_ApiRoot;
};

class EntityManager : public MutexClass
{
    void RetrieveEntityProfiles(Entity* pEntity);
public:
    EntityManager();
    ~EntityManager();

    int Initialize();
    int Shutdown();

    Entity* Discover(const std::string& entityurl);
private:
    std::vector<Entity*> m_Entities;


};

#endif

