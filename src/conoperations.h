
#ifndef CONOPERATIONS_H_
#define CONOPERATIONS_H_
#pragma once

#include "utils.h"
#include "connectionmanager.h"
#include "fileinfo.h"
#include "filemanager.h"
#include "atticpost.h"

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



};


#endif

