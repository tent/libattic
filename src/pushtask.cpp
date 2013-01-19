
#include "pushtask.h"

#include <list>

#include "filemanager.h"
#include "connectionmanager.h"
#include "chunkinfo.h"
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
        return ret::A_FAIL_INVALID_APP_INSTANCE;

    if(!GetFileManager())
        return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;

    // Index File
    std::string filename;
    utils::ExtractFileName(filepath, filename);

    std::cout << " Getting file info ... " << std::endl;

    while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0); } 
    FileInfo* fi = GetFileManager()->GetFileInfo(filename);
    GetFileManager()->Unlock();


    int status = ret::A_OK;
    if(!fi)
    {
        std::cout << "INDEXING FILE 0 : " << std::endl;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        status = GetFileManager()->IndexFileNew(filepath, true, NULL);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
            return status;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        fi = GetFileManager()->GetFileInfo(filename);
        GetFileManager()->Unlock();
    }
    else
    {
        std::cout << "INDEXING FILE 1 : " << std::endl;
        // Make sure temporary pieces exist
        // be able to pass in chosen chunkname
        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        status = GetFileManager()->IndexFileNew(filepath, true, fi);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
            return status;
    }

    // Create Chunk Post

    // Send Attic Post
    status = SendAtticPost(fi, filepath, filename);

    return status;
}


int PushTask::SendChunkPost( FileInfo* fi, 
                             const std::string& filepath, 
                             const std::string& filename )

{
    int status = ret::A_OK;
    // Create Chunk Post
    if(!fi)
        std::cout<<"invalid file info"<<std::endl;

    std::string chunkPostId;
    fi->GetChunkPostID(chunkPostId);

    // Get ChunkInfo List
    std::vector<ChunkInfo*>* pList = fi->GetChunkInfoList();

    // Construct post url
    // TODO :: abstract this common functionality somewhere else, utils?
    std::string posturl;
    GetEntity(posturl);
    posturl += "/tent/posts";

    if(chunkPostId.empty())
    {
        ChunkPost p;
        InitChunkPost(p, pList);
        // Post

    }
    else
    {
        // Put

    }


    return status;
}

int PushTask::SendAtticPost( FileInfo* fi, 
                             const std::string& filepath, 
                             const std::string& filename )
{
    int status = ret::A_OK;
    // Create Attic Post
    if(!fi)
        std::cout<<"invalid file info"<<std::endl;
    // Check for existing post
    std::string postid;
    fi->GetPostID(postid);

    // Get ChunkInfo List
    std::vector<ChunkInfo*>* pList = fi->GetChunkInfoList();

    // Construct post url
    // TODO :: abstract this common functionality somewhere else, utils?
    std::string posturl;
    GetEntity(posturl);
    posturl += "/tent/posts";

    Response response;
    if(postid.empty())
    {
        // New Post
        std::cout<< " POST URL : " << posturl << std::endl;

        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;
        InitAtticPost(p,
                      false,
                      filepath,
                      filename,
                      size,
                      pList);

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
                                   *at,
                                   response);
    }
    else
    {
        // Modify Post
        posturl += "/";
        posturl += postid;

        std::cout<< " PUT URL : " << posturl << std::endl;
        
        unsigned int size = utils::CheckFilesize(filepath);
        AtticPost p;
        InitAtticPost(p,
                      false,
                      filepath,
                      filename,
                      size,
                      pList);

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
                                  *at, response);
    }

    if(response.code != 200)
    {
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int PushTask::InitAtticPost( AtticPost& post,
                               bool pub,
                               const std::string& filepath,
                               const std::string& filename, 
                               unsigned int size,
                               std::vector<ChunkInfo*>* pList)
{
    int status = ret::A_OK;

    if(pList)
    {
        post.SetPermission(std::string("public"), pub);
        post.AtticPostSetFilepath(filepath);
        post.AtticPostSetFilename(filename);
        post.AtticPostSetSize(size);
        
        std::vector<ChunkInfo*>::iterator itr = pList->begin();

        std::string identifier, postids;
        for(;itr != pList->end(); itr++)
        {
            identifier.clear();
            postids.clear();

            if(*itr)
            {
                (*itr)->GetChecksum(identifier);

                


            }
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}


int PushTask::InitChunkPost(ChunkPost& post, std::vector<ChunkInfo*>* pList)
{
    int status = ret::A_OK;
    if(pList)
    {
        post.SetChunkInfoList(pList);
        

    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

