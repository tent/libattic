#include "deletetask.h"

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"


DeleteTask::DeleteTask( TentApp* pApp, 
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
                                  callback)
{

}

DeleteTask::~DeleteTask()
{

}

void DeleteTask::RunTask()
{
    // Run the task
    std::string filepath;
    GetFilepath(filepath);

    std::string filename;
    utils::ExtractFileName(filepath, filename);

    std::cout<<" FILE NAME : " << filename << std::endl;

    int status = DeleteFile(filename);

    // Callback
    Callback(status, NULL);
}


int DeleteTask::DeleteFile(const std::string& filename)
{
    if(!GetTentApp())
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!GetFileManager())
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0); } 
    FileInfo* fi = GetFileManager()->GetFileInfo(filename);
    GetFileManager()->Unlock();

    if(!fi)
        return ret::A_FAIL_FILE_NOT_IN_MANIFEST;

    std::string postid;
    fi->GetPostID(postid);

    int status = ret::A_OK;

    // Delete post
    status = DeletePost(postid);
    
    // Remove from Manifest
    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0); } 
    status = GetFileManager()->RemoveFile(filename);
    GetFileManager()->Unlock();

    return status; 
}

int DeleteTask::DeletePost(const std::string& szPostID)
{
    // Modify Post
    std::string posturl; 
    GetEntity(posturl);
    posturl += "/tent/posts/";
    posturl += szPostID;

    std::cout<< " DELETE URL : " << posturl << std::endl;

    AccessToken* at = GetAccessToken();
    std::string response;

    ConnectionManager::GetInstance()->HttpDelete( posturl,
                                                  NULL,
                                                  response,
                                                  at->GetMacAlgorithm(),
                                                  at->GetAccessToken(),
                                                  at->GetMacKey(),
                                                  true);

    std::cout<<"RESPONSE : " << response << std::endl;

    return ret::A_OK;
}   

