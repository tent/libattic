#include "deletetask.h"

#include "filemanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "conoperations.h"
#include "fileinfo.h"
#include "postutils.h"

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
    int status = ret::A_OK;

    std::string filepath;
    GetFilepath(filepath);
    FileInfo* fi = RetrieveFileInfo(filepath);
    // Mark as deleted
    status = MarkFileDeleted(fi);
    // Update Post
    status = UpdatePost(fi);
    
    // Callback
    Callback(status, NULL);
    SetFinishedState();
}

FileInfo* DeleteTask::RetrieveFileInfo(const std::string& filepath) {
    FileInfo* fi = NULL;
    FileManager* fm = GetFileManager();
    if(fm)
        fi = fm->GetFileInfo(filepath);

    return fi;
}

int DeleteTask::MarkFileDeleted(FileInfo* fi) {
    int status = ret::A_OK;
    if(fi) {
        std::string filepath;
        fi->GetFilepath(filepath);
        fi->SetDeleted(1);
        FileManager* fm = GetFileManager();
        if(fm)
            fm->SetFileDeleted(filepath, true);
    }
    else { 
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int DeleteTask::UpdatePost(FileInfo* fi) {
    int status = ret::A_OK;

    if(fi) {
        int trycount = 0;
        for(status = SendAtticPost(fi); status != ret::A_OK; trycount++) {
            status = SendAtticPost(fi);
            std::cout<<" RETRYING .................................." << std::endl;
            if(trycount > 2)
                break;
        }
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }
    return status;
}

int DeleteTask::SendAtticPost(FileInfo* fi) {
    int status = ret::A_OK;
    if(fi) {
        std::string postid;
        fi->GetPostID(postid);
        if(!postid.empty()) {
            std::string posturl;
            ConstructPostUrl(posturl);
            posturl += "/" + postid;

            std::cout<<" deleted : " << fi->GetDeleted() << std::endl;
            std::cout<<" delete url : " << postid << std::endl;

            AtticPost p;
            postutils::InitializeAtticPost(fi, p, false);

            std::string postBuffer;
            jsn::SerializeObject(&p, postBuffer);

            AccessToken* at = GetAccessToken();
            Response response;
            status = netlib::HttpPut( posturl,
                                      NULL,
                                      postBuffer,
                                      at,
                                      response);
            std::cout<<" response code : " << response.code << std::endl;
            std::cout<<" response body : " << response.body << std::endl;
            // Handle Response
            if(response.code != 200) {
                status = ret::A_FAIL_NON_200;
            }
        }
        else {
            std::cout<<" POST ID EMPTY INVALID FILEINFO " << std::endl;
            status = ret::A_FAIL_INVALID_POST_ID;
        }

    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;

}

// Depricated, kept for referece
int DeleteTask::DeletePost(const std::string& szPostID) {
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

