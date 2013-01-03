
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
            path += chunkPath + buf;

            out.push_back(path);
        }

        return ret::A_OK;
    }


    static int PostFile( const std::string& url, 
                         const std::string& filepath, 
                         const std::string& TempDirectory,
                         FileManager* fm,
                         ConnectionManager* cm,
                         FileInfo* fi,
                         Post* post,
                         AccessToken& at )
    {
        if(!fm)
            return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

        // Multipart post

        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        Response response;
        std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpMultipartPost( url, 
                                                             NULL,
                                                             postBuffer, 
                                                             &paths, 
                                                             response, 
                                                             at.GetMacAlgorithm(), 
                                                             at.GetAccessToken(), 
                                                             at.GetMacKey(), 
                                                             true);
        
        std::cout<<"CODE : " << response.code << std::endl;
        std::cout<<"RESPONSE : " << response.body << std::endl;

        AtticPost p;
        JsonSerializer::DeserializeObject(&p, response.body);

        int status = ret::A_OK;
        std::string postid;
        p.GetID(postid);
        if(!postid.empty())
        {
            fi->SetPostID(postid); 
            fi->SetPostVersion(0); // temporary for now, change later
            std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
            std::cout << " Name : " << (*p.GetAttachments())[0]->Name << std::endl;

            {
                while(fm->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
                std::string filename;
                fi->GetFilename(filename);
                fm->SetFilePostId(filename, postid);
                fm->Unlock();
            }

        }

        return status;
    }    

    static int PutFile( const std::string& url, 
                        const std::string& filepath, 
                        const std::string& TempDirectory,
                        FileManager* fm,
                        ConnectionManager* cm,
                        FileInfo* fi,
                        Post* post,
                        AccessToken& at )
    {    
        if(!fm)
            return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

        std::string postBuffer;
        JsonSerializer::SerializeObject(post, postBuffer);

        std::list<std::string> paths;
        AssembleChunkPaths(TempDirectory, fi, paths);

        Response response;
        std::cout<<" ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        ConnectionManager::GetInstance()->HttpMultipartPut( url, 
                                                            NULL,
                                                            postBuffer, 
                                                            &paths, 
                                                            response, 
                                                            at.GetMacAlgorithm(), 
                                                            at.GetAccessToken(), 
                                                            at.GetMacKey(), 
                                                            true);
     
        std::cout<<"CODE : " << response.code << std::endl;
        std::cout<<"RESPONSE : " << response.body << std::endl;

        AtticPost p;
        JsonSerializer::DeserializeObject(&p, response.body);

        std::string postid;
        p.GetID(postid);
        if(!postid.empty())
        {
           fi->SetPostID(postid); 
           fi->SetPostVersion(p.GetVersion()); // temporary for now, change later
           std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
           std::cout << " Name : " << (*p.GetAttachments())[0]->Name << std::endl;
        }

        return ret::A_OK;
    }    

    static int HttpPost( const std::string& url,
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
                                                    true);

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


};


#endif

