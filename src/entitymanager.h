
#ifndef ENTITYMANAGER_H_
#define ENTITYMANAGER_H_
#pragma once

#include <string>
#include <vector>

#include "mutexclass.h"
#include "accesstoken.h"

class Entity;

class EntityManager : public MutexClass
{
    void RetrieveEntityProfiles(Entity& ent, const AccessToken& at);
public:
    EntityManager();
    ~EntityManager();

    int Initialize();
    int Shutdown();

    int Discover(const std::string& entityurl, const AccessToken& at, Entity& entOut);
private:
    std::vector<Entity*> m_Entities; // TODO :: change this to a map, make it queryable


};
#endif

