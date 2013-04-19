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
                           const TaskContext& context,
                           TaskDelegate* callbackDelegate)
                           :                                               
                           TentTask( Task::SYNC_FILE_TASK,
                                     pFm,                                  
                                     pCm,                                  
                                     at,                                   
                                     entity,                               
                                     context,
                                     callbackDelegate) {
    // TODO :: refine constructor params, FOR NOW filepath will need to have the postid
    context.get_value("postid", post_id_);
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
    Callback(status, post_id_);
    SetFinishedState();
}

int SyncFileTask::SyncMetaData(FilePost& out) {
    int status = ret::A_OK;

    std::string url;
    utils::FindAndReplace(GetPostPath(), "{post}", post_id_, url);
    std::cout<<" post id : " << post_id_ << std::endl;
    std::cout<<" SYNC META DATA URL : " << url << std::endl;

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

    if(!p.deleted() && !p.in_transit()) {
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
            // compare file hashes
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

    std::string filepath = p.relative_path();
    if(!filepath.empty()) { 
        event::RaiseEvent(event::Event::REQUEST_PULL, filepath, NULL);
        processing_queue_[filepath] = true;
    }
    else {
        status = ret::A_FAIL_INVALID_FILEPATH;
    }

    return status;
}

}//namespace
