
#include "entity.h"

#include "errorcodes.h"
#include "connectionmanager.h"

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

}

void EntityManager::Discover(const std::string& entityurl)
{
    std::string response;
    ConnectionManager::GetInstance()->HttpGet(entityurl, NULL, response);

}

int EntityManager::Initialize()
{

    return ret::A_OK;
}

int EntityManager::Shutdown()
{

    return ret::A_OK;
}


