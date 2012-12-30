
#include "entity.h"

#include <vector>
#include <iostream> // TODO :: remove this

#include "utils.h"
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
    // Check entity url
    std::string profpath = entityurl;
    utils::CheckUrlAndAppendTrailingSlash(profpath);
    profpath += "profile";

    std::cout<<" discovering ... : " << profpath << std::endl;

    Response response;
    int code = ConnectionManager::GetInstance()->HttpGet(profpath, NULL, response);

    std::cout<< " resp : " << response.body << std::endl;

    std::cout<< " code : " << response.code << std::endl;

    if(response.code == 200)
    {
        std::vector<std::string> tags;
        // Parse out all link tags
        utils::FindAndExtractAllTags("link", response.body, tags);

        std::cout<< " size : " << tags.size() << std::endl;

//        for(int i = 0; i< tags.size() ; i++)
//            std::cout<< "\t" << tags[i] << std::endl;

        // Grab entity api root etc
    }
}

int EntityManager::Initialize()
{

    return ret::A_OK;
}

int EntityManager::Shutdown()
{

    return ret::A_OK;
}


