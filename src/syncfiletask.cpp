#include "syncfiletask.h"

#include <vector>

#include "response.h"
#include "urlparams.h"
#include "netlib.h"
#include "jsonserializable.h"
#include "postutils.h"
#include "entity.h"
#include "response.h"
#include "eventsystem.h"
#include "filesystem.h"


SyncFileTask::SyncFileTask( TentApp* pApp,
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
                            TentTask( Task::SYNC_FILE_TASK,
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
    // TODO :: refine constructor params, FOR NOW filepath will need to have the postid
    m_PostID = filepath;
}

SyncFileTask::~SyncFileTask()
{
}

void SyncFileTask::OnStart()
{
}

void SyncFileTask::OnPaused()
{
}

void SyncFileTask::OnFinished()
{
}

void SyncFileTask::RunTask()
{
    int status = ret::A_OK;

    // Retrieve metadata
    AtticPost p;
    status = SyncMetaData(p);
    if(status == ret::A_OK) {
        std::string filepath;
        p.GetAtticPostFilepath(filepath);

        FileInfo fi;
        postutils::DeserializeAtticPostIntoFileInfo(p, fi);

        // Check if file is in manifest
        int version = p.GetVersion();
        FileManager* fm = GetFileManager();

        // Get Local file info
        FileInfo* pLocal_fi = fm->GetFileInfo(filepath);
        std::string relative_path;
        fm->GetRelativeFilepath(filepath, relative_path);

        bool bPull = false;
        if(fm->DoesFileExist(filepath)) {
            // compare versions
            if(pLocal_fi->GetPostVersion() < version) {
                std::cout<<" VERSION : " << version << std::endl;
                // if version on the server is newer, pull
                bPull = true;
            }

            // check if file exists
            if(!fs::CheckFileExists(relative_path))
                bPull= true;
            else
                std::cout<<" FILE DOES NOT EXIST : " << relative_path << std::endl;

            
        }
        else {
            std::cout<<" file does not exist locally " << std::endl;
            bPull = true;
        }
        // Update and insert to manifest
        //
        if(bPull) {
            // retreive chunk info
            status = RetrieveChunkInfo(p, &fi);
            if(status == ret::A_OK) {
                // insert to file manager
                FileManager* fm = GetFileManager();
                fm->InsertToManifest(&fi);
                // pull request
                event::RaiseEvent(Event::REQUEST_PULL, relative_path, NULL);
            }
            else {
                std::cout<<" FAILED TO RETRIEVE CHUNK INFO " << std::endl;
            }
        }

    }

    Callback(status, NULL);
    SetFinishedState();
}

int Poll();
int SyncFileTask::SyncMetaData(AtticPost& out)
{
    int status = ret::A_OK;

    std::string url;
    GetEntityUrl(url);
    utils::CheckUrlAndAppendTrailingSlash(url);
    url += "posts/" + m_PostID;

    Response response;
    AccessToken* at = GetAccessToken();                                                
    netlib::HttpGet( url,
                     NULL,
                     at,
                     response); 

    std::cout<< " CODE : " << response.code << std::endl;
    std::cout<< " RESPONSE : " << response.body << std::endl;                          

    if(response.code == 200) {
        jsn::DeserializeObject(&out, response.body);
    }
    else
        status = ret::A_FAIL_NON_200;


    return status;
}

int SyncFileTask::RetrieveChunkInfo(AtticPost& post, FileInfo* fi)
{
    int status = ret::A_OK;

    Entity entity;
    GetEntity(entity);

    AccessToken* at = GetAccessToken();                                                
         
    // Get Chunk info
    std::vector<std::string> chunkPosts;
    chunkPosts = *post.GetChunkPosts();

    if(chunkPosts.size()) {
        std::cout<<" number of chunk posts : " << chunkPosts.size() << std::endl;
        std::cout<<" chunk post : " << chunkPosts[0] << std::endl;

        std::string chunkposturl;
        entity.GetApiRoot(chunkposturl);
        chunkposturl += "/posts/";

        std::vector<std::string>::iterator itr = chunkPosts.begin();
        std::string postid;

        for(;itr != chunkPosts.end(); itr++) {
            fi->SetChunkPostID(*itr);
            postid.clear();
            postid = *itr;
            std::string url = chunkposturl;
            url += postid;

            Response response;
            netlib::HttpGet( url, 
                             NULL,
                             at,
                             response); 

            std::cout<< " CODE : " << response.code << std::endl;
            //std::cout<< " RESP : " << response.body << std::endl;

            if(response.code == 200) {
                ChunkPost cp;
                jsn::DeserializeObject(&cp, response.body);
                if(cp.GetChunkSize()) {
                    std::cout<<" THIS ChunkPost : " << cp.GetChunkSize() << std::endl;
                    std::vector<ChunkInfo>* ciList = cp.GetChunkList();
                    std::vector<ChunkInfo>::iterator itr = ciList->begin();

                    for(;itr != ciList->end(); itr++) {
                        fi->PushChunkBack(*itr);
                    }

                }

                std::cout<<" CHUNK COUNT : " << fi->GetChunkCount() << std::endl;
                //fileInfoList.push_back(fi);
                //InsertFileInfoToManager(fileInfoList);
            }
        }
    }
    else {
        std::cout<<" NO CHUNKS ... : " << chunkPosts.size() << std::endl;
    }

    return status;
}
