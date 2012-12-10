
#include "pulltask.h"

#include <iostream>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"

PullTask::PullTask()
{
    m_pTentApp = NULL;
    m_pFileManager = NULL;
    m_pConnectionManager = NULL; 
}

PullTask::PullTask( TentApp* pApp, 
                    FileManager* pFm, 
                    ConnectionManager* pCon, 
                    const AccessToken& at,
                    const std::string& entity,
                    const std::string& filepath,
                    const std::string& tempdir)
{
    m_pTentApp = pApp;
    m_pFileManager = pFm;
    m_pConnectionManager = pCon; 

    m_At = at;

    m_Entity = entity;
    m_Filepath = filepath;
    m_TempDirectory = tempdir;

}

PullTask::~PullTask()
{

}

void PullTask::RunTask()
{

}

int PullTask::PullFile(const std::string& filepath)
{                                                                                                
    ////std::string filepath(szFilePath);                                                            
    std::string filename;                                                                        

    utils::ExtractFileName(filepath, filename);                                                  

    if(!m_pTentApp)                                                                                  
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;                                             

    if(!m_pFileManager)                                                                          
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;                                     

    if(!m_pConnectionManager)
        return ret::A_LIB_FAIL_INVALID_CONNECTIONMANAGER_INSTANCE;

    std::cout<<"FILE NAME : " << filename << std::endl;                                          

    FileInfo* fi = m_pFileManager->GetFileInfo(filename);                                        

    std::cout<<"HERE"<<std::endl;                                                                

    if(!fi)                                                                                      
    {                                                                                            
        std::cout<<"NULL FILE INFO"<<std::endl;                                                  

        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;                                                 
    }                                                                                            

    // Construct Post URL                                                                        
    std::string postpath = m_Entity;                                                             

    postpath.append("/tent/posts/");                                                             

    std::string postid;                                                                          
    fi->GetPostID(postid);                                                                       
    postpath += postid;                                                                          

    // Get Post                                                                                  
    std::string response;                                                                        
    ConnectionManager::GetInstance()->HttpGetWithAuth ( postpath,                                
                                                        NULL,                                    
                                                        response,                                
                                                        m_At.GetMacAlgorithm(),                  
                                                        m_At.GetAccessToken(),                   
                                                        m_At.GetMacKey(),                        
                                                        true);                                   


    std::cout << "RESPONSE : " << response << std::endl;                                         

    // Deserialize response into post                                                            
    Post resp;                                                                                   

    JsonSerializer::DeserializeObject(&resp, response);                                          

    std::cout << " Attachment Count : " << resp.GetAttachmentCount() << std::endl;               
    // Construct list of attachments                                                             

    Post::AttachmentVec* av = resp.GetAttachments();                                             
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
        outpath += m_TempDirectory;                                                          
        utils::CheckUrlAndAppendTrailingSlash(outpath);                                             
        outpath += (*itr)->Name;                                                             

        // Request attachment                                                                
        GetFileAndWriteOut(attachmentpath, outpath);                                         
    }                                                                                        

    // Construct File                                                                        
    std::cout << "STATUS : " << m_pFileManager->ConstructFile(filename) << std::endl;        

    return ret::A_OK;  
}

int PullTask::GetFileAndWriteOut(const std::string& url, const std::string &filepath)           
{                                                                                            
    // file path preferably to a chunked file.                                               
    if(!m_pTentApp)                                                                              
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;                                         

    ConnectionManager::GetInstance()->HttpGetAttachmentWriteToFile( url,                     
                                                                    NULL,                    
                                                                    filepath,                
                                                                    m_At.GetMacAlgorithm(),  
                                                                    m_At.GetAccessToken(),   
                                                                    m_At.GetMacKey(),
                                                                    true);                   

    return ret::A_OK;                                                                        
}                                                                                            

