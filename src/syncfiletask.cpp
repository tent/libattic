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
#include "taskdelegate.h"

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
                            const TaskDelegate* callbackDelegate)
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
                                      callbackDelegate)                             
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

void SyncFileTask::RunTask() {
    int status = ret::A_OK;
    std::cout << ".... Syncing file task ... " << std::endl;

    // Retrieve metadata
    AtticPost p;
    status = SyncMetaData(p);
    if(status == ret::A_OK) {
        status = ProcessFileInfo(p);
    }
    else {
        std::cout<<" ...failed to get metadata ... status : " << status << std::endl;
    }

    std::cout<<" ...sync file task finished ... " << std::endl;
    Callback(status, m_PostID);
    SetFinishedState();
}

int SyncFileTask::SyncMetaData(AtticPost& out) {
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

    if(response.code == 200) 
        jsn::DeserializeObject(&out, response.body);
    else
        status = ret::A_FAIL_NON_200;

    return status;
}

int SyncFileTask::ProcessFileInfo(const AtticPost& p) {
    int status = ret::A_OK;

    std::string filepath;
    p.GetAtticPostFilepath(filepath);
    std::cout<<" POST FILEPATH : " << filepath << std::endl;

    FileInfo fi;
    postutils::DeserializeAtticPostIntoFileInfo(p, fi);

    // Check if file is in manifest
    int version = p.GetVersion();
    FileManager* fm = GetFileManager();

    // Get Local file info
    FileInfo* pLocal_fi = fm->GetFileInfo(filepath);

    bool bPull = false;
    if(pLocal_fi) {
        std::string canonical_path;
        fm->GetCanonicalFilepath(filepath, canonical_path);

        std::cout<< "checking file....." << std::endl;
        // Is file marked as deleted?
        if(pLocal_fi->GetDeleted()) {
            std::cout<<" FILE DELETED " << std::endl;
            bPull = false;
        }
        // compare versions
        else if(pLocal_fi->GetPostVersion() < version) {
            std::cout<<" VERSION : " << version << std::endl;
            std::cout<<" LOCAL VERSION " << pLocal_fi->GetPostVersion() << std::endl;
            // if version on the server is newer, pull
            bPull = true;
        }
        // check if file exists
        else if(!fs::CheckFileExists(canonical_path)) { 
            std::cout<<" checking if file exists --- " << std::endl;
            std::cout<<" maybe use path ? : " << filepath << std::endl;
            std::cout<<" or ? : " << canonical_path << std::endl;
            std::cout<<" file does not exist pulling ... " << std::endl;
            bPull= true;
        }

        std::cout<<" pullling ? : " << bPull << std::endl;
        // Update and insert to manifest
    }
    else {
        std::cout<< " NULL local file info " << std::endl;
        std::cout<< " just pull ... " << std::endl;
        bPull = true;
    }

    if(bPull) {
        // retreive chunk info
        status = RetrieveChunkInfo(p, &fi);
        if(status == ret::A_OK) {
            std::cout<<" GET FILEINFO VERSION : " << fi.GetPostVersion() << std::endl;
            std::cout<<" GET POST VERSION : " << version << std::endl;
            std::cout<<" INSERTING " << std::endl;

            // insert to file manager
            FileManager* fm = GetFileManager();
            fm->InsertToManifest(&fi);
            // pull request
            event::RaiseEvent(event::Event::REQUEST_PULL, filepath, NULL);
            m_ProcessingQueue[filepath] = true;
        }
        else {
            std::cout<<" FAILED TO RETRIEVE CHUNK INFO " << std::endl;
        }
    }
    else {
        std::cout<<" not pulling ... " << std::endl;
    }

    return status;
}

int SyncFileTask::RetrieveChunkInfo(const AtticPost& post, FileInfo* fi) {
    int status = ret::A_OK;

    Entity entity;
    GetEntity(entity);

    AccessToken* at = GetAccessToken();                                                
         
    // Get Chunk info
    std::vector<std::string> chunkPosts;
    chunkPosts = post.GetChunkPosts();

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

            std::cout<<" getting : " << url << std::endl;
            Response response;
            netlib::HttpGet( url, 
                             NULL,
                             at,
                             response); 

            //std::cout<< " CODE : " << response.code << std::endl;
            //std::cout<< " RESP : " << response.body << std::endl;

            if(response.code == 200) {
                ChunkPost cp;
                jsn::DeserializeObject(&cp, response.body);

                if(cp.GetChunkSize()) {
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
