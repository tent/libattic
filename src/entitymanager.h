
#ifndef ENTITYMANAGER_H_
#define ENTITYMANAGER_H_
#pragma once

#include <string>
#include <vector>
#include "mutexclass.h"

class Entity;

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
    std::vector<Entity*> m_Entities; // TODO :: change this to a map, make it queryable


};
#endif

