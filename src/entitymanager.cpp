#include "entitymanager.h"

#include <iostream> // TODO :: remove this

#include "entity.h"
#include "utils.h"
#include "constants.h"
#include "errorcodes.h"
#include "conoperations.h"

#include "log.h"

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

}

int EntityManager::Initialize()
{
    // Load entity info from somewhere ... if it exists

    return ret::A_OK;
}

int EntityManager::Shutdown()
{
    // Clear out Entity vector


    return ret::A_OK;
}


int EntityManager::Discover(const std::string& entityurl, const AccessToken& at, Entity& entOut)
{
    std::cout<< " discover " << std::endl;
    int status = ret::A_OK;
    status = conops::DiscoverWithAuth(entityurl, at, entOut);

    std::cout<< " discover " << std::endl;

    if(status == ret::A_OK)
    {
    std::cout<< " discover " << std::endl;
        // Grab entity api root etc
        RetrieveEntityProfiles(entOut, at);
        
    std::cout<< " discover " << std::endl;
        // Set Api root
        Profile* pProf = entOut.GetActiveProfile();
    std::cout<< " discover " << std::endl;
        if(pProf)
        {
            std::string apiroot;
            pProf->GetApiRoot(apiroot);
            entOut.SetApiRoot(apiroot);
            entOut.SetEntityUrl(entityurl);
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }
    else
    {
        alog::Log(Logger::ERROR, std::string("entitymanager->discover") +
                  std::string("ERROR : ") + ret::ErrorToString(status));
    }

    return status; 
}

void EntityManager::RetrieveEntityProfiles(Entity& ent, const AccessToken& at)
{
    unsigned int profcount = ent.GetProfileCount();
    if(profcount)
    {
        const Entity::UrlList* ProfUrlList = ent.GetProfileUrlList();
        Entity::UrlList::const_iterator itr = ProfUrlList->begin();

        while(itr != ProfUrlList->end())
        {
            Response response;
            conops::HttpGet( *itr, 
                             NULL,
                             at,
                             response);

            /*
            std::cout<< " resp : " << response.body << std::endl;
            std::cout<< " code : " << response.code << std::endl;
            */
 
            if(response.code == 200)
            {
                // Deserialize into Profile Object
                Profile* pProf = new Profile();
                JsonSerializer::DeserializeObject(pProf, response.body);
                
                // Push back into entity
                ent.PushBackProfile(pProf);
            }
            itr++;
        }

        Entity::ProfileList* pProfList = ent.GetProfileList();
        if(pProfList)
            ent.SetActiveProfile(&*pProfList->begin());
   }
}
