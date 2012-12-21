
#ifndef ENTITY_H_
#define ENTITY_H_
#pragma once

#include <string>

class Entity
{
public:
    Entity();
    ~Entity();

    void GetEntityUrl(std::string& out) const { out = m_EntityUrl; }
    void GetApiRoot(std::string& out) const { out = m_ApiRoot; }
    
    void SetEntityUrl(const std::string& url) { m_EntityUrl = url; }
    void SetApiRoot(const std::string& root) { m_ApiRoot = root; }

private:
    std::string     m_EntityUrl;
    std::string     m_ApiRoot;
};



class EntityManager
{
public:
    EntityManager();
    ~EntityManager();

    void Discover(const std::string& entityurl);


};

#endif

