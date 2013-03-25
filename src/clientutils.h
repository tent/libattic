#ifndef CLIENTUTILS_H_
#define CLIENTUTILS_H_ 
#pragma once

#include <string>

#include "utils.h"
#include "netlib.h"
#include "constants.h"
#include "entity.h"
#include "accesstoken.h"
#include "profile.h"

namespace client {

static void RetrieveEntityProfiles(const AccessToken* at, Entity& ent) {
    unsigned int profcount = ent.GetProfileCount();
    if(profcount) {
        const Entity::UrlList* ProfUrlList = ent.GetProfileUrlList();
        Entity::UrlList::const_iterator itr = ProfUrlList->begin();

        std::cout<<" profile list size : " << ProfUrlList->size() << std::endl;
        if(at)
            std::cout<<" TOKEN : " << at->GetAccessToken() << std::endl;

        while(itr != ProfUrlList->end()) {
            Response response;
            
            netlib::HttpGet( *itr, 
                             NULL,
                             at,
                             response);
 
            if(response.code == 200) {
                std::cout<<" RETRIEVE ENTITY PROFILES " << std::endl;
                std::cout<<" code : " << response.code << std::endl;
                std::cout<<" body : " << response.body << std::endl;
                // Deserialize into Profile Object
                Profile* pProf = new Profile();
                jsn::DeserializeObject(pProf, response.body);
                
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

static int ExtractProfile( const std::string& entityurl, 
                           Response& response, 
                           Entity& entOut)
{
    int status = ret::A_OK;

    if(response.code == 200) {
        utils::split s;
        utils::SplitString(response.body, '\n', s);

        utils::split::iterator itr = s.begin();
        for(;itr != s.end(); itr++) {
            size_t pos = (*itr).find("Link");
            
            if(pos != (size_t)-1) {
                pos = (*itr).find("<");

                if(pos != (size_t)-1) {
                    size_t end = (*itr).find(">", pos+1);
                    size_t diff = (end - (pos+1));
                    std::string str = (*itr).substr(pos+1, diff);

                    std::cout<<" DIFF : " << str << std::endl;

                    std::string url;
                    if(str.find(entityurl) == (size_t)-1) {
                        std::string eurl = entityurl;
                        utils::CheckUrlAndRemoveTrailingSlash(eurl);
                        url += eurl + str;
                    }
                    else
                        url = str;

                    std::cout<<" URL : " << url << std::endl;
                    entOut.PushBackProfileUrl(url);
                }
            }
        }
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

static int HeadRequestEntity(const std::string& entityurl, Entity& entOut) {
    int status = ret::A_OK;

    Response response;
    netlib::HttpHead( entityurl, NULL, NULL, response);
    
    if(response.code == 200) {
        status = ExtractProfile(entityurl, response, entOut);
    }
    else { 
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

static int GetRequestEntity(const std::string& entityurl, Entity& entOut) {
    int status = ret::A_OK;

    Response response;
    netlib::HttpGet( entityurl,
                     NULL,
                     NULL, //at
                     response); 

    std::cout<<" GET REQUEST ENTITY " << std::endl;
    std::cout<<" CODE : " << response.code << std::endl;
    std::cout<<" BODY :"  << response.body << std::endl;
    if(response.code == 200) {
        // Parse out all link tags
        utils::taglist tags;
        utils::FindAndExtractAllTags("link", response.body, tags);

        std::string str;
        for(int i = 0; i< tags.size() ; i++) {
            //std::cout<< "\t" << tags[i] << std::endl;
            size_t found = tags[i].find(cnst::g_szProfileRel);

            if(found != std::string::npos)
            {
                // Extract Profile url
                size_t b = tags[i].find("https");
                size_t e = tags[i].find("\"", b+1);
                size_t diff = e - b;

                str.clear();
                str = tags[i].substr(b, diff);

                entOut.PushBackProfileUrl(str);
            }
        }
    }
    else {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

static int InitEntity(const std::string& entityurl, const AccessToken* at, Entity& entOut) {
    int status = ret::A_OK;

    // Grab entity api root etc
    RetrieveEntityProfiles(at, entOut);
    
    // Set Api root
    Profile* pProf = entOut.GetActiveProfile();
    if(pProf) {
        std::string apiroot;
        pProf->GetApiRoot(apiroot);
        entOut.SetApiRoot(apiroot);
        entOut.SetEntityUrl(entityurl);
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

static int Discover(const std::string& entityurl, const AccessToken* at, Entity& entOut) {
    std::cout<<" Discovering entity ... " << std::endl;
    std::cout<<" entity url : " << entityurl << std::endl;
    int status = ret::A_OK;

    status = HeadRequestEntity(entityurl,entOut);
    if(status != ret::A_OK) { 
        std::cout<<" head request failed ... get request " << std::endl;
        status = GetRequestEntity(entityurl, entOut);
    }

    if(status == ret::A_OK) {
        std::cout<<" Init entity " << std::endl;
        status = InitEntity(entityurl, at, entOut);
    }

    std::cout<<" Discover status : " << status << std::endl;
    return status; 
}


}
#endif

