#include "entitymanager.h"

#include <iostream> // TODO :: remove this

#include "entity.h"
#include "utils.h"
#include "constants.h"
#include "errorcodes.h"
#include "conoperations.h"


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
    // Check entity url
    std::string profpath = entityurl;
    utils::CheckUrlAndAppendTrailingSlash(profpath);
    profpath += "profile";

    std::cout<<" discovering ... : " << profpath << std::endl;

    Response response;
    conops::HttpGet( profpath, 
                     NULL,
                     at,
                     response );

    if(response.code == 200)
    {

        // Parse out all link tags
        utils::taglist tags;
        utils::FindAndExtractAllTags("link", response.body, tags);

        std::string str;
        for(int i = 0; i< tags.size() ; i++)
        {
            std::cout<< "\t" << tags[i] << std::endl;
            size_t found = tags[i].find(cnst::g_szProfileRel);

            if(found != std::string::npos)
            {
                std::cout<< " FOUND : " << i << std::endl;
                // Extract Profile url
                size_t b = tags[i].find("https");
                size_t e = tags[i].find("\"", b+1);
                size_t diff = e - b;

                str.clear();
                str = tags[i].substr(b, diff);
                std::cout<<" SUBSTR : " << str << std::endl;

                entOut.PushBackProfileUrl(str);

                /*
                Response resp;
                conops::HttpGet( str, 
                                 NULL,
                                 at,
                                 resp);
    
            //    std::cout<< " resp : " << resp.body << std::endl;
                std::cout<< " code : " << resp.code << std::endl;
                */
            }
        }

        // Grab entity api root etc
        RetrieveEntityProfiles(entOut, at);
    }

    ret::A_OK;
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

            std::cout<< " resp : " << response.body << std::endl;
            std::cout<< " code : " << response.code << std::endl;
 
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
    }
}
