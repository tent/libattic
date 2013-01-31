
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
                    TentTask ( pApp,
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

PushTask::~PushTask()
{

}

void PushTask::RunTask()
{
    std::cout<<" TASK IS RUNNING " << std::endl;
    // Run the task
    std::string filepath;
    GetFilepath(filepath);

    int status = PushFile(filepath);
    
    // Callback
    Callback(status, NULL);
    SetFinishedState();
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

    GetFileManager()->Lock();
    FileInfo* fi = GetFileManager()->GetFileInfo(filepath);
    GetFileManager()->Unlock();

    int status = ret::A_OK;
    if(!fi)
    {
        std::cout << "INDEXING FILE 0 : " << std::endl;

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        status = GetFileManager()->IndexFileNew(filepath, true, NULL);
        GetFileManager()->Unlock();

        if(status != ret::A_OK)
        {
            std::cout<<" failed to index : " << status << std::endl;
            std::cout<<" filepath : " << filepath << std::endl;
            return status;
        }

        while(GetFileManager()->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
        fi = GetFileManager()->GetFileInfo(filepath);
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

    if(status == ret::A_OK)
    {
        // Create Chunk Post
        int trycount = 0;
        for(status = SendChunkPost(fi, filepath, filename); status != ret::A_OK; trycount++)
        {
            status = SendChunkPost(fi, filepath, filename);
            std::cout<<" RETRYING .................................." << std::endl;
            if(trycount > 2)
                break;
        }
    }

    if(status == ret::A_OK)
    {
        // Send Attic Post
        int trycount = 0;
        for(status = SendAtticPost(fi, filepath, filename); status != ret::A_OK; trycount++)
        {
            status = SendAtticPost(fi, filepath, filename);
            std::cout<<" RETRYING .................................." << std::endl;
            if(trycount > 2)
                break;
        }
    }

    std::cout << "finishing up " << std::endl;

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
    FileInfo::ChunkMap* pList = fi->GetChunkInfoList();

    std::cout<<" CHUNK LIST SIZE : " << pList->size() << std::endl;


    // Construct post url
    // TODO :: abstract this common functionality somewhere else, utils?
    std::string posturl;
    Entity entity;
    GetEntity(entity);
    entity.GetApiRoot(posturl);
    posturl += "/posts";

    bool post = true;
    Response response;
    if(chunkPostId.empty())
    {
        ChunkPost p;
        InitChunkPost(p, *pList);
        // Post
        std::string tempdir;
        GetTempDirectory(tempdir);

        AccessToken* at = GetAccessToken();
        status = conops::PostFile( posturl, 
                                   filepath, 
                                   tempdir, 
                                   ConnectionManager::GetInstance(), 
                                   fi,
                                   &p,
                                   *at,
                                   response);
    }
    else
    {
        // Put
        post = false;
        // Modify Post
        posturl += "/";
        posturl += chunkPostId;

        std::cout<< " PUT URL : " << posturl << std::endl;
        
        unsigned int size = utils::CheckFilesize(filepath);

        ChunkPost p;
        InitChunkPost(p, *pList);

        std::string tempdir;
        GetTempDirectory(tempdir);

        AccessToken* at = GetAccessToken();
        status = conops::PutFile( posturl, 
                                  filepath, 
                                  tempdir, 
                                  ConnectionManager::GetInstance(), 
                                  fi,
                                  &p,
                                  *at, response);
    }

    // Handle Response
    if(response.code == 200)
    {
        std::cout<<" HANDLING SUCCESSFUL RESPONSE : " << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;

        ChunkPost p;
        JsonSerializer::DeserializeObject(&p, response.body);

        std::string postid;
        p.GetID(postid);

        if(!postid.empty())
        {
            fi->SetChunkPostID(postid); 
            fi->SetPostVersion(0); // temporary for now, change later
            std::cout << " SIZE : " << p.GetAttachments()->size() << std::endl;
            std::cout << " Name : " << (*p.GetAttachments())[0].Name << std::endl;

            FileManager* fm = GetFileManager();
            if(post)
            {
                fm->Lock();
                fm->SetFileChunkPostId(filepath, postid);
                fm->Unlock();
            }
        }
    }
    else
    {
        status = ret::A_FAIL_NON_200;
    }

    std::cout<< " RESPONSE : " << response.code << std::endl;
    std::cout<< " BODY : " << response.body << std::endl;

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
    FileInfo::ChunkMap* pList = fi->GetChunkInfoList();

    std::cout<<" CHUUUUUUUUUNK LIST SIZE : " << pList->size() << std::endl;

    // Construct post url
    // TODO :: abstract this common functionality somewhere else, utils?

    Entity entity;
    GetEntity(entity);
    std::string posturl;
    entity.GetApiRoot(posturl);
    posturl += "/posts";

    std::string chunkname;
    fi->GetChunkName(chunkname);

    bool post = true;
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
                      chunkname,
                      size,
                      pList);

        std::string postBuffer;
        JsonSerializer::SerializeObject(&p, postBuffer);

        std::cout<<"\n\n Attic Post Buffer : " << postBuffer << std::endl;

        AccessToken* at = GetAccessToken();

        status = conops::HttpPost( posturl,
                                   NULL,
                                   postBuffer,
                                   *at,
                                   response );
    }
    else
    {
        post = false;
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
                      chunkname,
                      size,
                      pList);

        std::string postBuffer;
        JsonSerializer::SerializeObject(&p, postBuffer);

        std::cout<<"\n\n Attic Post Buffer : " << postBuffer << std::endl;

        AccessToken* at = GetAccessToken();
        status = conops::HttpPut( posturl,
                                   NULL,
                                   postBuffer,
                                   *at,
                                   response );
   }

    // Handle Response
    if(response.code == 200)
    {
        std::cout<<" HANDLING SUCCESSFUL RESPONSE : " << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;

        AtticPost p;
        JsonSerializer::DeserializeObject(&p, response.body);

        std::string postid;
        p.GetID(postid);

        if(!postid.empty())
        {
            FileManager* fm = GetFileManager();
            fi->SetPostID(postid); 
            if(post)
            {
                std::string filepath;
                fi->SetPostVersion(0); // temporary for now, change later
                fi->GetFilepath(filepath);

                while(fm->TryLock()) { /* Spinlock, temporary */ sleep(0);} 
                fm->SetFilePostId(filepath, postid);
                fm->Unlock();
            }
        }
    }
    else
    {
        std::cout<<" HANDLING FAILED RESPONSE : " << response.code << std::endl;
        std::cout<<" BODY : " << response.body << std::endl;


        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int PushTask::InitAtticPost( AtticPost& post,
                             bool pub,
                             const std::string& filepath,
                             const std::string& filename, 
                             const std::string& chunkname,
                             unsigned int size,
                             FileInfo::ChunkMap* pList)
{
    int status = ret::A_OK;

    if(pList)
    {
        post.SetPermission(std::string("public"), pub);
        post.AtticPostSetFilepath(filepath);
        post.AtticPostSetFilename(filename);
        post.AtticPostSetSize(size);
        post.AtticPostSetChunkName(chunkname);
        
        FileInfo::ChunkMap::iterator itr = pList->begin();

        std::string identifier, postids;
        for(;itr != pList->end(); itr++)
        {
            identifier.clear();
            postids.clear();

            itr->second.GetChecksum(identifier);
            post.PushBackChunkIdentifier(identifier);
        }

        FileManager* fm = GetFileManager();
        fm->Lock(); 
        FileInfo* fi = fm->GetFileInfo(filepath);
        fm->Unlock();

        std::string chunkpostid;
        if(fi)
        {
            std::string encKey, iv;
            fi->GetEncryptedKey(encKey);
            fi->GetIv(iv);

            post.AtticPostSetKeyData(encKey);
            post.AtticPostSetIvData(iv);

            fi->GetChunkPostID(chunkpostid);
            post.PushBackChunkPostId(chunkpostid);
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}


int PushTask::InitChunkPost(ChunkPost& post, FileInfo::ChunkMap& List)
{
    int status = ret::A_OK;

    post.SetChunkInfoList(List);

    return status;
}

