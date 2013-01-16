
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
    fi->GetPostID(postid);                                                                       
    postpath += postid;                                                                          

    std::cout<<" POST ID : " << postid << std::endl;

    // Get Post                                                                                  
    AccessToken* at = GetAccessToken();
    Response response;                                                                        
    ConnectionManager::GetInstance()->HttpGetWithAuth ( postpath,                                
                                                        NULL,                                    
                                                        response,                                
                                                        at->GetMacAlgorithm(),                  
                                                        at->GetAccessToken(),                   
                                                        at->GetMacKey(),                        
                                                        true);                                   


    std::cout << "CODE : " << response.code << std::endl;
    std::cout << "RESPONSE : " << response.body << std::endl;                                         

    if(response.code == 200)
    {
        // Deserialize response into post                                                            
        Post resp;                                                                                   

        JsonSerializer::DeserializeObject(&resp, response.body);                                          

        std::cout << " Attachment Count : " << resp.GetAttachmentCount() << std::endl;               
        // Construct list of attachments                                                             

        Post::AttachmentVec* av = resp.GetAttachments();                                             

        std::cout<< " VEC COUNT : " << av->size() << std::endl;
        Post::AttachmentVec::iterator itr = av->begin();                                             

        std::string attachmentpath;                                                                  
        std::string outpath;                                                                         

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

        // Construct File                                                                        
        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);}
//        std::cout << "STATUS : " << GetFileManager()->ConstructFile(filename) << std::endl;        
        std::cout << " constructing file ... " << std::endl;
        std::cout << " CONSTRUCT STATUS : " << GetFileManager()->ConstructFileNew(filename) << std::endl;        
        GetFileManager()->Unlock();
    }
    else
    {
        return ret::A_FAIL_NON_200;
    }

    return ret::A_OK;  
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

