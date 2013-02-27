#include "deletetask.h"

#include "filemanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "conoperations.h"


DeleteTask::DeleteTask( TentApp* pApp, 
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
                        TentTask( Task::DELETE,
                                  pApp,
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
    SetFinishedState();
}


int DeleteTask::DeleteFile(const std::string& filename)
{
    if(!GetTentApp())
        return ret::A_FAIL_INVALID_APP_INSTANCE;

    if(!GetFileManager())
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;

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
    status = GetFileManager()->RemoveFile(filename);

    return status; 
}

int DeleteTask::DeletePost(const std::string& szPostID)
{
    // Modify Post
    Entity entity;
    GetEntity(entity);

    std::string posturl; 
    entity.GetApiRoot(posturl);
    posturl += "/posts/";
    posturl += szPostID;

    std::cout<< " DELETE URL : " << posturl << std::endl;
    AccessToken* at = GetAccessToken();

    Response response;
    netlib::HttpDelete( posturl,
                        NULL,
                        at,
                        response);

    std::cout<<"Code : " << response.code << std::endl;
    std::cout<<"RESPONSE : " << response.body << std::endl;

    return ret::A_OK;
}   

