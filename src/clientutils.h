#ifndef CLIENTUTILS_H_
#define CLIENTUTILS_H_ 
#pragma once

#include <string>

#include "utils.h"
#include "netlib.h"
#include "constants.h"
#include "entity.h"
#include "entitypost.h"
#include "accesstoken.h"

namespace client {

static int HeadRequestEntity(const std::string& entityurl, std::string& linkOut);
static void ExtractMetaLink(Response& response, std::string& linkOut);
static void ExtractLink(const std::string& link, std::string& out);

static void ConstructMetaPath(const std::string& entityurl,
                              const std::string& metaendpoint,
                              std::string& out);



    // UPDATE ALL OF THIS V03
static void RetrieveEntityProfiles(const AccessToken* at, Entity& ent) {
    /*
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
   */
}
static int Discover(const std::string& entityurl, const AccessToken* at, Entity& entOut) {
    int status = ret::A_OK;

    std::string meta_link;
    std::cout<<" head request : " << entityurl << std::endl;
    status = HeadRequestEntity(entityurl, meta_link);
    std::cout<<" META LINK : " << meta_link << std::endl;
    if(status == ret::A_OK) {
        Response response;
        netlib::HttpGet(meta_link, NULL, NULL, response);

        if(response.code == 200) {
            std::cout<<" CODE : " << response.code << std::endl;
            std::cout<<" BODY : " << response.body << std::endl;
            EntityPost ep;
            jsn::DeserializeObject(&ep, response.body);

            std::cout<<" here " << std::endl;
            entOut = ep.entity();

            Json::Value val;
            jsn::SerializeObject(&entOut, val);
            jsn::PrintOutJsonValue(&val);

        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }
    

    std::cout<<" Discover status : " << status << std::endl;
    return status; 
}

static int HeadRequestEntity(const std::string& entityurl, std::string& linkOut) {
    int status = ret::A_OK;

    Response response;
    netlib::HttpHead(entityurl, NULL, NULL, response);

    if(response.code == 200) {
        std::string link;
        ExtractMetaLink(response, link);
        std::string metapath;
        ConstructMetaPath(entityurl, link, metapath);
        linkOut = metapath;
    }
    else { 
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

static void ConstructMetaPath(const std::string& entityurl,
                              const std::string& metaendpoint,
                              std::string& out) {
    // Check if already absolute
    size_t ent_pos = metaendpoint.find(entityurl);
    if(ent_pos != std::string::npos && ent_pos != 0) {
        // Construct meta path
        out = entityurl;
        utils::CheckUrlAndRemoveTrailingSlash(out);
        out += metaendpoint;
    }
    else {
        out = metaendpoint;
    }
}

static void ExtractMetaLink(Response& response, std::string& linkOut) {
    std::string link_header = response.header["Link"];
    if(!link_header.empty()){
        if(link_header.find(cnst::g_meta_rel) != std::string::npos)
            ExtractLink(link_header, linkOut);
    }
}

static void ExtractLink(const std::string& link, std::string& out) {
    size_t begin = link.find("<");
    size_t end = link.find(">");
    size_t diff = (end - (begin+1));
    out = link.substr(begin+1, diff);
}

static int InitEntity(const std::string& entityurl, const AccessToken* at, Entity& entOut) {
    int status = ret::A_OK;

    // Grab entity api root etc
    RetrieveEntityProfiles(at, entOut);
    
    /*
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
    */

    return status;
}




}
#endif

