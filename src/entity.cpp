
#include "entity.h"

#include "errorcodes.h"
#include "connectionmanager.h"

#include <iostream>

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

}

void EntityManager::Discover(const std::string& entityurl)
{
    Response response;
    int code = ConnectionManager::GetInstance()->HttpGet(entityurl, NULL, response);

    std::cout<< " resp : " << response.body << std::endl;

    std::cout<< " code : " << response.code << std::endl;

}

int EntityManager::Initialize()
{

    return ret::A_OK;
}

int EntityManager::Shutdown()
{

    return ret::A_OK;
}


