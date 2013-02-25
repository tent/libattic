
#ifndef CONOPERATIONS_H_
#define CONOPERATIONS_H_
#pragma once

#include "utils.h"
#include "connectionmanager.h"
#include "fileinfo.h"
#include "filemanager.h"
#include "atticpost.h"
#include "constants.h"
#include "multipartconsumer.h"

#include "netlib.h"

namespace conops
{
    //////////// Basic connection operations
    static int HttpPost( const std::string& url,
                         const UrlParams* pParams,
                         const std::string& body,
                         AccessToken& at,
                         Response& responseOut)
    {

        ConnectionManager::GetInstance()->HttpPostWithAuth( url,
                                                           pParams,
                                                           body,
                                                           responseOut,
                                                           at.GetMacAlgorithm(), 
                                                           at.GetAccessToken(), 
                                                           at.GetMacKey(), 
                                                           false);

        return ret::A_OK;
    }

    static int HttpPut( const std::string& url,
                         const UrlParams* pParams,
                         const std::string& body,
                         AccessToken& at,
                         Response& responseOut)
    {

        ConnectionManager::GetInstance()->HttpPutWithAuth( url,
                                                           pParams,
                                                           body,
                                                           responseOut,
                                                           at.GetMacAlgorithm(), 
                                                           at.GetAccessToken(), 
                                                           at.GetMacKey(), 
                                                           false);

        return ret::A_OK;
    }

    static int HttpGet( const std::string& url,
                        const UrlParams* pParams,
                        const AccessToken& at,
                        Response& responseOut)
    {
        ConnectionManager::GetInstance()->HttpGetWithAuth( url,                               
                                                           pParams,
                                                           responseOut,
                                                           at.GetMacAlgorithm(),          
                                                           at.GetAccessToken(),           
                                                           at.GetMacKey(),
                                                           false);
        return ret::A_OK;
    }

    static int HttpGetAttachmentAndWriteOut( const std::string& url,
                                             const UrlParams* pParams,
                                             const AccessToken& at,
                                             const std::string& filepath,
                                             Response& responseOut)
    {
        int status = netlib::HttpGetAttachment( url, pParams, &at, responseOut);
        /*
        ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                    
                                                                        pParams,                   
                                                                        responseOut,               
                                                                        filepath,               
                                                                        at.GetMacAlgorithm(),  
                                                                        at.GetAccessToken(),   
                                                                        at.GetMacKey(),        
                                                                        false);                  
        */


        return status;
    } 

    static int HttpGetAttachmentAndWriteOut( const std::string& url,
                                             const UrlParams* pParams,
                                             const AccessToken& at,
                                             const std::string& filepath,
                                             ConnectionHandle* pHandler,
                                             Response& responseOut)
    {

        int status = ret::A_OK;
        std::cout<<" ATTACHMENT URL : " << url << std::endl;

        //status = netlib::HttpGetAttachment( url, pParams, &at, responseOut);

        status = netlib::HttpAsioGetAttachment( url, pParams, &at, responseOut);


        /*
        ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                    
                                                                        pParams,                   
                                                                        responseOut,               
                                                                        filepath,               
                                                                        at.GetMacAlgorithm(),  
                                                                        at.GetAccessToken(),   
                                                                        at.GetMacKey(),        
                                                                        pHandler,
                                                                        false);                  

        */


        std::cout<< " GET ATTACH RESPONSE CODE : " << responseOut.code << std::endl;
        std::cout<< " GET ATTACH RESPONSE BODY : " << responseOut.body << std::endl;

        return status;
    }
    static int HttpDelete( const std::string& url,
                           const UrlParams* pParams,
                           const AccessToken& at,
                           Response& responseOut)
    {
        ConnectionManager::GetInstance()->HttpDelete( url,
                                                      pParams,
                                                      responseOut,
                                                      at.GetMacAlgorithm(),
                                                      at.GetAccessToken(),
                                                      at.GetMacKey(),
                                                      false);

        return ret::A_OK;
    }

    static int HttpHead( const std::string& url, 
                         const UrlParams* pParams,
                         Response& responseOut)
    {
        int status = ret::A_OK;

        ConnectionManager::GetInstance()->HttpHead( url,
                                                    pParams,
                                                    responseOut,
                                                    true); 
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
    static int PutFile( const std::string& url, 
                        const std::string& filepath, 
                        const std::string& TempDirectory,
                        FileInfo* fi,
                        Post* post,
                        AccessToken& at,
                        ConnectionHandle* ch,
                        Response& responseOut)
    {    

        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        netlib::HttpAsioMultipartRequest("PUT", url, NULL, postBuffer, &at, paths, responseOut);
        return 0;

/*
        ConnectionManager::GetInstance()->HttpMultipartPut( url, 
                                                             NULL,
                                                             postBuffer, 
                                                             &paths, 
                                                             responseOut, 
                                                             at.GetMacAlgorithm(), 
                                                             at.GetAccessToken(), 
                                                             at.GetMacKey(), 
                                                             ch,
                                                             false); 
                                                             */
        return ret::A_OK;
    }



    static int PostFile( const std::string& url, 
                         const std::string& filepath, 
                         const std::string& TempDirectory,
                         FileInfo* fi,
                         Post* post,
                         AccessToken& at,
                         ConnectionHandle* ch,
                         Response& responseOut )
    {
        // Multipart post
        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        netlib::HttpAsioMultipartRequest("POST", url, NULL, postBuffer, &at, paths, responseOut);
        return 0;

        /*
        std::cout<<" Sending new post! " << std::endl;
        std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                             NULL,
                                                             postBuffer, 
                                                             &paths, 
                                                             responseOut, 
                                                             at.GetMacAlgorithm(), 
                                                             at.GetAccessToken(), 
                                                             at.GetMacKey(), 
                                                             ch,
                                                             false);
        
        std::cout<<"CODE : " << responseOut.code << std::endl;
        std::cout<<"RESPONSE : " << responseOut.body << std::endl;
        */

        return ret::A_OK;
    }

    static int PostFile( const std::string& url, 
                         const std::string& filepath, 
                         const std::string& TempDirectory,
                         ConnectionManager* cm,
                         FileInfo* fi,
                         Post* post,
                         AccessToken& at,
                         Response& responseOut )
    {
        // Multipart post
        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                             NULL,
                                                             postBuffer, 
                                                             &paths, 
                                                             responseOut, 
                                                             at.GetMacAlgorithm(), 
                                                             at.GetAccessToken(), 
                                                             at.GetMacKey(), 
                                                             false);
        
        std::cout<<"CODE : " << responseOut.code << std::endl;
        std::cout<<"RESPONSE : " << responseOut.body << std::endl;

        return ret::A_OK;
    }    

    static int PutFile( const std::string& url, 
                        const std::string& filepath, 
                        const std::string& TempDirectory,
                        ConnectionManager* cm,
                        FileInfo* fi,
                        Post* post,
                        AccessToken& at,
                        Response& responseOut)
    {    

        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpMultipartPut( url, 
                                                            NULL,
                                                            postBuffer, 
                                                            &paths, 
                                                            responseOut, 
                                                            at.GetMacAlgorithm(), 
                                                            at.GetAccessToken(), 
                                                            at.GetMacKey(), 
                                                            false);
     
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
                /*
                conops::HttpGet( *itr, 
                                 NULL,
                                 at,
                                 response);
                                 */

                ConnectionManager::GetInstance()->HttpGet( *itr,
                                                           NULL,
                                                           response,
                                                           true);

                
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

            Entity::ProfileList* pProfList = ent.GetProfileList();
            if(pProfList)
                ent.SetActiveProfile(&*pProfList->begin());
       }
    }

    static void RetrieveEntityProfiles(const AccessToken& at, Entity& ent)
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

                HttpGet( *itr, 
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
        HttpHead( entityurl, NULL, response);
    
        status = ExtractProfile(entityurl, response, entOut);
        return status;
    }

    static int HeadRequestEntityWithAuth( const std::string& entityurl, 
                                          const AccessToken& at, 
                                          Entity& entOut)
    {
        int status = ret::A_OK;

        Response response;
        std::cout<<" ENTITY URL : " << entityurl << std::endl;
        std::cout<< at.GetMacAlgorithm() << " " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpHeadWithAuth( entityurl,
                                                    NULL,
                                                    response,
                                                    at.GetMacAlgorithm(), 
                                                    at.GetAccessToken(), 
                                                    at.GetMacKey(),
                                                    true); 


        status = ExtractProfile(entityurl, response, entOut);
        return status;
    }

    static int GetRequestEntity(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        Response response;
        ConnectionManager::GetInstance()->HttpGet( entityurl,
                                                    NULL,
                                                    response,
                                                    true); 

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

            /*
            // Grab entity api root etc
            RetrieveEntityProfiles(entOut);
            
            // Set Api root
            Profile* pProf = entOut.GetActiveProfile();
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
            */

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

        return status;
    }

    static int InitEntity(const std::string& entityurl, const AccessToken& at, Entity& entOut)
    {
        int status = ret::A_OK;

        // Grab entity api root etc
            RetrieveEntityProfiles(at, entOut);
            
            // Set Api root
            Profile* pProf = entOut.GetActiveProfile();
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

        return status;
    }


    static int Discover(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        status = HeadRequestEntity(entityurl,entOut);
        if(status != ret::A_OK)
            status = GetRequestEntity(entityurl, entOut);

        if(status == ret::A_OK)
            status = InitEntity(entityurl, entOut);

        return status; 
    }

    static int DiscoverWithAuth(const std::string& entityurl, const AccessToken& at, Entity& entOut)
    {
        int status = ret::A_OK;


        status = HeadRequestEntity(entityurl,entOut);
        //status = HeadRequestEntityWithAuth( entityurl,
         //                                   at,
          //                                  entOut);
        if(status != ret::A_OK)
            status = GetRequestEntity(entityurl, entOut);

        if(status == ret::A_OK)
            status = InitEntity(entityurl, at, entOut);

        return status;
    }





};


#endif

