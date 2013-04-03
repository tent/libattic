#include "syncfiletask.h"

#include <vector>

#include "response.h"
#include "urlparams.h"
#include "netlib.h"
#include "jsonserializable.h"
#include "postutils.h"
#include "entity.h"
#include "response.h"
#include "event.h"
#include "filesystem.h"
#include "taskdelegate.h"
#include "logutils.h"
#include "configmanager.h"

namespace attic { 

SyncFileTask::SyncFileTask(FileManager* pFm,
                           CredentialsManager* pCm,
                           const AccessToken& at,
                           const Entity& entity,
                           const std::string& filepath,
                           const std::string& tempdir,
                           const std::string& workingdir,
                           const std::string& configdir,
                           TaskDelegate* callbackDelegate)
                           :                                               
                           TentTask( Task::SYNC_FILE_TASK,
                                     pFm,                                  
                                     pCm,                                  
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

SyncFileTask::~SyncFileTask() {}
void SyncFileTask::OnStart() {}
void SyncFileTask::OnPaused() {}
void SyncFileTask::OnFinished() {}

void SyncFileTask::RunTask() {
    int status = ret::A_OK;
    // Retrieve metadata
    FilePost p;
    status = SyncMetaData(p);
    if(status == ret::A_OK) {
        try {
            status = ProcessFileInfo(p);
        }
        catch(std::exception &e) {
            log::LogException("UJaoe3234", e);
        }
    }
    else {
        std::cout<<" ...failed to get metadata ... status : " << status << std::endl;
    }

    //std::cout<<" ...sync file task finished ... " << std::endl;
    Callback(status, m_PostID);
    SetFinishedState();
}

int SyncFileTask::SyncMetaData(FilePost& out) {
    int status = ret::A_OK;

    std::string url;
    utils::FindAndReplace(GetPostPath(), "{post}", m_PostID, url);

    //std::cout<<" SYNC META DATA URL : " << url << std::endl;

    Response response;
    AccessToken at = access_token();
    netlib::HttpGet( url,
                     NULL,
                     &at,
                     response); 

    if(response.code == 200) 
        jsn::DeserializeObject(&out, response.body);
    else
        status = ret::A_FAIL_NON_200;

    return status;
}

int SyncFileTask::ProcessFileInfo(const FilePost& p) {
    int status = ret::A_OK;
    FileManager* fm = GetFileManager();
    if(!fm) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    std::string filepath = p.relative_path();

    if(!p.deleted()) {
        FileInfo fi;
        postutils::DeserializeFilePostIntoFileInfo(p, fi);
        // Check if file is in manifest
        //int version = p.GetVersion();

        // Get Local file info
        FileInfo* pLocal_fi = fm->GetFileInfo(filepath);

        bool bPull = false;
        if(pLocal_fi) {
            std::string canonical_path;
            fm->GetCanonicalFilepath(filepath, canonical_path);

            if(pLocal_fi->deleted())
                bPull = false;
            //TODO VO3
            // compare versions
            // check if file exists, locally
            else if( !fs::CheckFilepathExists(canonical_path))
                bPull= true;
        }
        else {
            // Doesn't exist in the manifest
            bPull = true;
        }
        if(bPull) RaisePullRequest(p, fi);
    }
    else {
        std::string canonical_path;
        fm->GetCanonicalFilepath(filepath, canonical_path);
        if(fs::CheckFilepathExists(canonical_path)){
            // Move to trash
            std::string trash_path;
            ConfigManager::GetInstance()->GetValue("trash_path", trash_path);
            if(!trash_path.empty() && fs::CheckFilepathExists(trash_path)) {
                // Move to trash;
                fs::MoveFile(canonical_path, trash_path);
            }
            else {
                std::string msg = "Invalid trash_path";
                log::LogString("MOA1349", msg);
            }
        }
    }

    return status;
}

int SyncFileTask::RaisePullRequest(const FilePost& p, FileInfo& fi) {
    int status = ret::A_OK;

    // retreive chunk info
    status = RetrieveChunkInfo(p, fi);
    if(status == ret::A_OK) {
        std::string filepath = p.relative_path();
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

    return status;
}

int SyncFileTask::RetrieveChunkInfo(const FilePost& post, FileInfo& fi) {
    int status = ret::A_OK;

    Entity entity = TentTask::entity();
    AccessToken at = access_token();                                                
         
    // Get Chunk info
    std::vector<std::string> chunkPosts;
    chunkPosts = post.GetChunkPosts();

    if(chunkPosts.size()) {
        std::vector<std::string>::iterator itr = chunkPosts.begin();
        std::string postid;
        for(;itr != chunkPosts.end(); itr++) {
            fi.set_chunk_post_id(*itr);
            postid.clear();
            postid = *itr;
            std::string url;
            utils::FindAndReplace(GetPostPath(), "{post}", postid, url);

            Response response;
            netlib::HttpGet(url, 
                            NULL,
                            &at,
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
                        fi.PushChunkBack(*itr);
                    }
                }
            }
            else {
                status = ret::A_FAIL_NON_200;
                log::LogHttpResponse("MNB889RFA", response);
            }
        }
    }
    else {
        status = ret::A_FAIL_EMPTY_CHUNK_POST;
    }

    return status;
}

}//namespace
