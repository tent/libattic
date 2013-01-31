#include "pulltask.h"

#include <iostream>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "conoperations.h"


PullTask::PullTask( TentApp* pApp, 
                    FileManager* pFm, 
                    CredentialsManager* pCm,
                    TaskArbiter* pTa,
                    TaskFactory* pTf,
                    const AccessToken& at,
                    const Entity& entity,
                    const std::string& filepath,
                    const std::string& tempdir,
                    const std::string& workingdir,
                    const std::string& configdir,
                    void (*callback)(int, void*))
                    :
                    TentTask( pApp,
                              pFm,
                              pCm,
                              pTa,
                              pTf,
                              at,
                              entity,
                              filepath,
                              tempdir,
                              workingdir,
                              configdir,
                              callback )
{

}

PullTask::~PullTask()
{

}

void PullTask::RunTask()
{
    std::cout<<" Running pull task " << std::endl;
    std::string filepath;
    GetFilepath(filepath);
    int status = PullFile(filepath);

    Callback(status, NULL);
    SetFinishedState();
}

int PullTask::PullFile(const std::string& filepath)
{                                                                                                
    //std::string filename;                                                                        
    //utils::ExtractFileName(filepath, filename);                                                  

    if(!GetFileManager())                                                                          
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;                                     

    GetFileManager()->Lock();
    FileInfo* fi = GetFileManager()->GetFileInfo(filepath);                                        
    GetFileManager()->Unlock();

    if(!fi)                                                                                      
    {                                                                                            
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }                                                                                            

    // Construct Post URL                                                                        
    std::string postpath;// = m_Entity;                                                             
    GetEntityUrl(postpath);
    postpath.append("/tent/posts/");                                                             

    std::string postid;                                                                          
    //fi->GetPostID(postid);                                                                       
    fi->GetChunkPostID(postid);
    postpath += postid;                                                                          

    int status = ret::A_OK;
    Response response;                                                                        
    status = GetChunkPost(fi, response);

    if(status == ret::A_OK)
    {
        /*
        std::cout<<" Chunk post response : " << response.code << std::endl;
        std::cout<<" Chunk post body : " << response.body << std::endl;
        */

        if(response.code == 200)
        {
            // Deserialize response into post
            Post resp;
            JsonSerializer::DeserializeObject(&resp, response.body);
            GetAttachmentsFromPost(postpath, resp);

            // Construct File                                                                        
            GetFileManager()->Lock();
            GetFileManager()->ConstructFileNew(filepath);
            GetFileManager()->Unlock();
        }
        else
        {
            status = ret::A_FAIL_NON_200;
        }
    }

    return status;
}

int PullTask::GetChunkPost(FileInfo* fi, Response& responseOut)
{
    int status = ret::A_OK;

    if(fi)
    {
        // Construct Post URL                                                                        
        std::string postpath;// = m_Entity;
        GetEntityUrl(postpath);
        postpath.append("/tent/posts/");                                                             

        std::string postid;                                                            
        //fi->GetPostID(postid);
        fi->GetChunkPostID(postid);
        postpath += postid;                                                                          

        std::cout<<" Post path : " << postpath << std::endl;
        // Get Post                                                                                  
        AccessToken* at = GetAccessToken();
        if(at)
        {
            status = conops::HttpGet(postpath, NULL, *at, responseOut);

            //std::cout<<" response out : " << responseOut.body << std::endl;
            if(status == ret::A_OK)
            {
                if(responseOut.code != 200)
                    status = ret::A_FAIL_NON_200;
            }
        }
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int PullTask::GetAttachmentsFromPost(const std::string postpath, Post& post)
{
    int status = ret::A_OK;

    // Construct list of attachments                                                             
    Post::AttachmentVec* av = post.GetAttachments();                                             
    Post::AttachmentVec::iterator itr = av->begin();                                             
    std::string attachmentpath, outpath;

    for(;itr != av->end(); itr++)                                                            
    {                                                                                        
        // Construct attachment path                                                         
        attachmentpath.clear();                                                              
        attachmentpath += postpath;                                                          
        attachmentpath.append("/attachments/");                                              
        attachmentpath += (*itr).Name;                                                      

        outpath.clear();                                                                     
        GetTempDirectory(outpath);

        utils::CheckUrlAndAppendTrailingSlash(outpath);                                             
        outpath += (*itr).Name;                                                             

        // Request attachment                                                                
        GetFileAndWriteOut(attachmentpath, outpath);                                         
    }                                                                                        
    
    return status;
}

int PullTask::GetFileAndWriteOut(const std::string& url, const std::string &filepath)           
{                                                                                            
    int status = ret::A_OK;

    Response response;
    AccessToken* at = GetAccessToken();

    if(at)
    {
        status = conops::HttpGetAttachmentAndWriteOut(url, NULL, *at, filepath, response);
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;                                                                        
}                                                                                            

