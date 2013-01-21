
#include "pulltask.h"

#include <iostream>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"


PullTask::PullTask( TentApp* pApp, 
                    FileManager* pFm, 
                    ConnectionManager* pCon, 
                    CredentialsManager* pCm,
                    const AccessToken& at,
                    const std::string& entity,
                    const std::string& filepath,
                    const std::string& tempdir,
                    const std::string& workingdir,
                    const std::string& configdir,
                    void (*callback)(int, void*))
                    :
                    TentTask( pApp,
                              pFm,
                              pCon,
                              pCm,
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
    std::cout<<" RUNNING TASK " << std::endl;

    std::string filepath;
    GetFilepath(filepath);
    int status = PullFile(filepath);

    Callback(status, NULL);
}

int PullTask::PullFile(const std::string& filepath)
{                                                                                                
    std::string filename;                                                                        
    utils::ExtractFileName(filepath, filename);                                                  

    if(!GetTentApp())                                                                                  
        return ret::A_FAIL_INVALID_APP_INSTANCE;                                             

    if(!GetFileManager())                                                                          
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;                                     

    if(!GetConnectionManager())
        return ret::A_FAIL_INVALID_CONNECTIONMANAGER_INSTANCE;

    std::cout<<"FILE NAME : " << filename << std::endl;                                          

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);}
    FileInfo* fi = GetFileManager()->GetFileInfo(filename);                                        
    GetFileManager()->Unlock();

    std::cout<<"HERE"<<std::endl;                                                                

    if(!fi)                                                                                      
    {                                                                                            
        std::cout<<"NULL FILE INFO"<<std::endl;                                                  
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }                                                                                            


    // Construct Post URL                                                                        
    std::string postpath;// = m_Entity;                                                             
    GetEntity(postpath);

    postpath.append("/tent/posts/");                                                             

    std::string postid;                                                                          
    //fi->GetPostID(postid);                                                                       
    fi->GetChunkPostID(postid);
    postpath += postid;                                                                          

    Response response;                                                                        
    GetChunkPost(fi, response);

    if(response.code == 200)
    {
        // Deserialize response into post                                                            
        Post resp;                                                                                   
        JsonSerializer::DeserializeObject(&resp, response.body);                                          

        GetAttachmentsFromPost(postpath, resp);

        // Construct File                                                                        
        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);}
        GetFileManager()->ConstructFileNew(filename);
        GetFileManager()->Unlock();
    }
    else
    {
        return ret::A_FAIL_NON_200;
    }

    return ret::A_OK;  
}

int PullTask::GetChunkPost(FileInfo* fi, Response& responseOut)
{
    int status = ret::A_OK;

    if(fi)
    {
        // Construct Post URL                                                                        
        std::string postpath;// = m_Entity;                                                             
        GetEntity(postpath);

        postpath.append("/tent/posts/");                                                             

        std::string postid;                                                                          
        //fi->GetPostID(postid);                                                                       
        fi->GetChunkPostID(postid);
        postpath += postid;                                                                          

        std::cout<<" POST ID : " << postid << std::endl;

        // Get Post                                                                                  
        AccessToken* at = GetAccessToken();
        ConnectionManager::GetInstance()->HttpGetWithAuth ( postpath,                                
                                                            NULL,                                    
                                                            responseOut,                                
                                                            at->GetMacAlgorithm(),                  
                                                            at->GetAccessToken(),                   
                                                            at->GetMacKey(),                        
                                                            true);                                   


        std::cout << "CODE : " << responseOut.code << std::endl;
        std::cout << "RESPONSE : " << responseOut.body << std::endl;                                         

        if(responseOut.code != 200)
            status = ret::A_FAIL_NON_200;
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

    std::cout<< " VEC COUNT : " << av->size() << std::endl;
    Post::AttachmentVec::iterator itr = av->begin();                                             

    std::string attachmentpath, outpath;                                                                         

    for(;itr != av->end(); itr++)                                                            
    {                                                                                        
        // Construct attachment path                                                         
        attachmentpath.clear();                                                              
        attachmentpath += postpath;                                                          
        attachmentpath.append("/attachments/");                                              
        attachmentpath += (*itr)->Name;                                                      
        std::cout<< attachmentpath << std::endl;                                             

        outpath.clear();                                                                     
        GetTempDirectory(outpath);

        utils::CheckUrlAndAppendTrailingSlash(outpath);                                             
        outpath += (*itr)->Name;                                                             

        std::cout<<" NAME : " <<  (*itr)->Name << std::endl;

        // Request attachment                                                                
        GetFileAndWriteOut(attachmentpath, outpath);                                         
    }                                                                                        

    
    return status;
}

int PullTask::GetFileAndWriteOut(const std::string& url, const std::string &filepath)           
{                                                                                            
    // file path preferably to a chunked file.                                               
    if(!GetTentApp())                                                                              
        return ret::A_FAIL_INVALID_APP_INSTANCE;                                         

    Response response;
    AccessToken* at = GetAccessToken();
    ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                     
                                                                    NULL,                    
                                                                    response,
                                                                    filepath,                
                                                                    at->GetMacAlgorithm(),  
                                                                    at->GetAccessToken(),   
                                                                    at->GetMacKey(),
                                                                    true);                   

    return ret::A_OK;                                                                        
}                                                                                            

