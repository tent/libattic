
#include "pushtask.h"

#include <list>

#include "filemanager.h"
#include "connectionmanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "constants.h"

#include "conoperations.h"

PushTask::PushTask( TentApp* pApp, 
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
                    TentTask ( pApp,
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

PushTask::~PushTask()
{

}

void PushTask::RunTask()
{
    // Run the task
    std::string filepath;
    GetFilepath(filepath);

    int status = PushFile(filepath);
    // Callback
    Callback(status, NULL);
}

int PushTask::PushFile(const std::string& filepath)
{
    if(!GetTentApp())
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!GetFileManager())
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    std::string filename;
    utils::ExtractFileName(filepath, filename);

    std::cout << " Getting file info ... " << std::endl;

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0); } 
    FileInfo* fi = GetFileManager()->GetFileInfo(filename);
    GetFileManager()->Unlock();


    int status = ret::A_OK;
    if(!fi)
    {


        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        std::cout << "INDEXING FILE : " << std::endl;
        status = GetFileManager()->IndexFile(filepath, true);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
            return status;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        fi = GetFileManager()->GetFileInfo(filename);
        GetFileManager()->Unlock();
    }
    else
    {
        // Make sure temporary pieces exist
        // be able to pass in chosen chunkname
        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        std::cout << "INDEXING FILE : " << std::endl;
        int status = GetFileManager()->IndexFile(filepath, true, fi);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
            return status;
    }

    // Check for existing post
    std::string postid;
    fi->GetPostID(postid);

    // Construct post url
    // TODO :: abstract this common functionality somewhere else, utils?
    std::string posturl;
    GetEntity(posturl);
    posturl += "/tent/posts";

    if(postid.empty())
    {
        // New Post
        std::cout<< " POST URL : " << posturl << std::endl;

        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;
        CreateAtticPost(p,
                        false,
                        filepath,
                        filename,
                        size);

        std::string tempdir;
        GetTempDirectory(tempdir);

        AccessToken* at = GetAccessToken();
        status = conops::PostFile( posturl, 
                                   filepath, 
                                   tempdir, 
                                   GetFileManager(), 
                                   GetConnectionManager(), 
                                   fi,
                                   &p,
                                   *at);
    }
    else
    {
        // Modify Post
        posturl += "/";
        posturl += postid;

        std::cout<< " PUT URL : " << posturl << std::endl;
        
        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;
        CreateAtticPost(p,
                        false,
                        filepath,
                        filename,
                        size);

        std::string tempdir;
        GetTempDirectory(tempdir);

        AccessToken* at = GetAccessToken();
        status = conops::PutFile( posturl, 
                                  filepath, 
                                  tempdir, 
                                  GetFileManager(), 
                                  GetConnectionManager(), 
                                  fi,
                                  &p,
                                  *at);
    }

    return status;
}

int PushTask::CreateAtticPost( AtticPost& post,
                               bool pub,
                               const std::string& filepath,
                               const std::string& filename, 
                               unsigned int size)
{
    post.SetPermission(std::string("public"), pub);
    post.AtticPostSetFilepath(filepath);
    post.AtticPostSetFilename(filename);
    post.AtticPostSetSize(size);

    return ret::A_OK;
}


