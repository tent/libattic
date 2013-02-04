
#ifndef CONOPERATIONS_H_
#define CONOPERATIONS_H_
#pragma once

#include "utils.h"
#include "connectionmanager.h"
#include "fileinfo.h"
#include "filemanager.h"
#include "atticpost.h"
#include "constants.h"

namespace conops
{
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
        ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                    
                                                                        pParams,                   
                                                                        responseOut,               
                                                                        filepath,               
                                                                        at.GetMacAlgorithm(),  
                                                                        at.GetAccessToken(),   
                                                                        at.GetMacKey(),        
                                                                        false);                  


        return ret::A_OK;
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

    static void RetrieveEntityProfiles(Entity& ent)
    {
        unsigned int profcount = ent.GetProfileCount();
        if(profcount)
        {
            const Entity::UrlList* ProfUrlList = ent.GetProfileUrlList();
            Entity::UrlList::const_iterator itr = ProfUrlList->begin();

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

    static int Discover(const std::string& entityurl, Entity& entOut)
    {
        int status = ret::A_OK;

        // Check entity url
        std::string profpath = entityurl;
        utils::CheckUrlAndAppendTrailingSlash(profpath);
        profpath += "profile";

        Response response;
        ConnectionManager::GetInstance()->HttpGet( profpath,
                                                   NULL,
                                                   response,
                                                   true); 
        std::cout<<" body : " << response.body << std::endl;
        if(response.code == 200)
        {

            // Parse out all link tags
            utils::taglist tags;
            utils::FindAndExtractAllTags("link", response.body, tags);

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

        }
        else
        {
            status = ret::A_FAIL_NON_200;
        }

        return status; 
    }


};


#endif

