
#ifndef CONOPERATIONS_H_
#define CONOPERATIONS_H_
#pragma once

#include "utils.h"
#include "fileinfo.h"
#include "filemanager.h"
#include "atticpost.h"
#include "constants.h"
#include "multipartconsumer.h"

#include "netlib.h"

namespace conops
{
    static int HttpGetAttachmentAndWriteOut( const std::string& url,
                                             const UrlParams* pParams,
                                             const AccessToken& at,
                                             const std::string& filepath,
                                             Response& responseOut)
    {
        int status = netlib::HttpGetAttachment( url, pParams, &at, responseOut);
        return status;
    } 

    static int AssembleChunkPaths( const std::string& dir, 
                                   const FileInfo* fi, 
                                   std::list<std::string>& out)
    {

        // This may be abstracted
        // construct chunk filepaths
        std::string chunkName; 
        fi->GetChunkName(chunkName);
        std::string chunkPath = dir;

        // Assemble chunk paths list
        chunkPath.append("/");
        chunkPath.append(chunkName);
        chunkPath.append("_");

        std::string path;
        char buf[256];

        for(unsigned int i=0; i< fi->GetChunkCount(); i++)
        {
            memset(buf, '\0', 256);
            snprintf(buf, 256, "%u", i);

            path.clear();
            path += chunkPath + buf + "_enc";

            out.push_back(path);
        }

        return ret::A_OK;
    }

    static void RetrieveEntityProfiles(Entity& ent)
    {
        unsigned int profcount = ent.GetProfileCount();
        std::cout<<" PROF COUNT : " << profcount << std::endl;
        if(profcount)
        {
            const Entity::UrlList* ProfUrlList = ent.GetProfileUrlList();
            Entity::UrlList::const_iterator itr = ProfUrlList->begin();

            std::cout<<" profile list size : " << ProfUrlList->size() << std::endl;

            while(itr != ProfUrlList->end())
            {
                Response response;
                
                netlib::HttpGet( *itr, 
                                 NULL,
                                 NULL, //at,
                                 response);

                std::cout<< " resp : " << response.body << std::endl;
                std::cout<< " code : " << response.code << std::endl;
                
     
                if(response.code == 200)
                {
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

    static void RetrieveEntityProfiles(const AccessToken& at, Entity& ent)
    {
        std::cout<<" REEEEEEETREIVE ENTITY PROFILES < -------------- " << std::endl;
        unsigned int profcount = ent.GetProfileCount();
        std::cout<<" PROF COUNT : " << profcount << std::endl;
        if(profcount) {
            const Entity::UrlList* ProfUrlList = ent.GetProfileUrlList();
            Entity::UrlList::const_iterator itr = ProfUrlList->begin();

            std::cout<<" profile list size : " << ProfUrlList->size() << std::endl;

            while(itr != ProfUrlList->end()) {
                Response response;

                std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;

                netlib::HttpGet( *itr, 
                                 NULL,
                                 &at,
                                 response);

                std::cout<< " resp : " << response.body << std::endl;
                std::cout<< " code : " << response.code << std::endl;
                
     
                if(response.code == 200)
                {
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

        if(response.code == 200)
        {
            std::cout<<" RESPONSE : " << response.body << std::endl;

            utils::split s;
            utils::SplitString(response.body, '\n', s);

            std::cout<<" SIZE : " << s.size() << std::endl;

            utils::split::iterator itr = s.begin();
            for(;itr != s.end(); itr++)
            {
                size_t pos = (*itr).find("Link");
                std::cout<< " POS : " << pos << std::endl;
                
                if(pos != (size_t)-1)
                {
                    pos = (*itr).find("<");

                    if(pos != (size_t)-1)
                    {
                        size_t end = (*itr).find(">", pos+1);
                        size_t diff = (end - (pos+1));
                        std::string str = (*itr).substr(pos+1, diff);

                        std::cout<<" DIFF : " << str << std::endl;

                        std::string url;
                        if(str.find(entityurl) == (size_t)-1)
                        {
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
        else
        {
            status = ret::A_FAIL_NON_200;
        }

        return status;
    }

    

    static int HeadRequestEntity(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        Response response;
        std::cout<<" calling ... " << std::endl;
        netlib::HttpHead( entityurl, NULL, NULL, response);

        std::cout<<" RESPONSE : " << response.code << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;
        
        if(response.code == 200) {
            status = ExtractProfile(entityurl, response, entOut);
        }
        else { 
            status = ret::A_FAIL_NON_200;
        }

        return status;
    }
    static int GetRequestEntity(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        Response response;
        netlib::HttpGet( entityurl,
                         NULL,
                         NULL, //at
                         response); 

        if(response.code == 200)
        {
            // Parse out all link tags
            utils::taglist tags;
            utils::FindAndExtractAllTags("link", response.body, tags);

            std::cout<<" TAG SIZE : " << tags.size() << std::endl;
            std::string str;
            for(int i = 0; i< tags.size() ; i++)
            {
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
        else
        {
            status = ret::A_FAIL_NON_200;
        }

        return status;

    }

    static int InitEntity(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        // Grab entity api root etc
            RetrieveEntityProfiles(entOut);
            
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

    static int InitEntity(const std::string& entityurl, const AccessToken& at, Entity& entOut)
    {
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


    static int Discover(const std::string& entityurl, Entity& entOut)
    {
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
            status = InitEntity(entityurl, entOut);
        }

        std::cout<<" Discover status : " << status << std::endl;
        return status; 
    }

    static int DiscoverWithAuth(const std::string& entityurl, const AccessToken& at, Entity& entOut)
    {
        int status = ret::A_OK;

        std::cout<<" Discovering entity with auth... " << std::endl;
        std::cout<<" entity url : " << entityurl << std::endl;

        status = HeadRequestEntity(entityurl,entOut);
        //status = HeadRequestEntityWithAuth( entityurl,
         //                                   at,
          //                                  entOut);
        if(status != ret::A_OK) { 
            std::cout<<" head request failed ... get request " << std::endl;
            status = GetRequestEntity(entityurl, entOut);
        }

        if(status == ret::A_OK) { 
            std::cout<<" Init entity " << std::endl;
            status = InitEntity(entityurl, at, entOut);
        }

        std::cout<<" Discover with auth status : " << status << std::endl;

        return status;
    }





};


#endif

