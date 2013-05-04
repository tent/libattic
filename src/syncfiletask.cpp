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
                           const TaskContext& context)
                           :                                               
                           TentTask( Task::SYNC_FILE_TASK,
                                     pFm,                                  
                                     pCm,                                  
                                     at,                                   
                                     entity,                               
                                     context) {
    // TODO :: refine constructor params, FOR NOW filepath will need to have the postid
    context.get_value("postid", post_id_);
}

SyncFileTask::~SyncFileTask() {}
void SyncFileTask::OnStart() {}
void SyncFileTask::OnPaused() {}
void SyncFileTask::OnFinished() {}

void SyncFileTask::RunTask() {
    int status = ret::A_OK;
    std::cout<<" Syncing post id : " << post_id_ << std::endl;
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
    FileManager* fm = file_manager();
    if(!fm) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    std::string filepath = p.relative_path();

    if(!p.deleted()) { 
        if(!p.in_transit()) {
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

                // Check if any aliases exist, and fix
                CheckForAliases(p, filepath);
                
                // check if file exists, locally
                if(pLocal_fi->deleted())
                    bPull = false;
                else if(!fs::CheckFilepathExists(canonical_path))
                    bPull= true;
            }
            else {
                std::cout<<" NOT IN MANIFEST PULL " << std::endl;
                // Insert into manifest
                fm->InsertToManifest(&fi);
                // Doesn't exist in the manifest
                bPull = true;
            }
            if(bPull) RaisePullRequest(p, fi);
        }
        else {
            std::cout<<" file in transit " << std::endl;
        }
    }
    else {
        std::cout<<" file marked for deletion " << std::endl;
        std::string canonical_path;
        fm->GetCanonicalFilepath(filepath, canonical_path);
        if(fs::CheckFilepathExists(canonical_path)){
            // Move to trash
            std::string trash_path;
            ConfigManager::GetInstance()->GetValue("trash_path", trash_path);
            if(!trash_path.empty() && fs::CheckFilepathExists(trash_path)) {
                // Move to trash;
                fs::MoveFileToFolder(canonical_path, trash_path);
            }
            else {
                std::string msg = "Invalid trash_path";
                log::LogString("MOA1349", msg);
            }
        }
    }

    return status;
}

void SyncFileTask::CheckForAliases(const FilePost& p, const std::string& filepath) {
    std::cout<<" CHECK FOR ALIASES " << std::endl;
    std::cout<<" FILEEEEEEE PATH : " << filepath << std::endl;
    FileManager* fm = file_manager();
    std::vector<std::string> aliases = p.GetPastAliases();

    std::vector<std::string>::iterator itr = aliases.begin();
    for(;itr != aliases.end(); itr++) {
        std::string canonical;
        std::cout<<" PURE ALIAS : " << (*itr) << std::endl;
        fm->GetCanonicalFilepath((*itr), canonical);
        std::cout<<" CHECKING ALIAS : " << canonical << std::endl;
        if(fs::CheckFilepathExists(canonical)) {
            std::cout<<" OLD PATH EXISTS! RENAME IT! " << std::endl;
            std::string new_filepath;
            fm->GetCanonicalFilepath(filepath, new_filepath);

            try {
                std::cout<<" canonical : " << canonical << std::endl;
                std::cout<<" new filepath : " << new_filepath << std::endl;
                fs::RenamePath(canonical, new_filepath);
            }
            catch (std::exception& e) {
                std::cout<<" RENAME EXCEPTION : " << e.what() << std::endl;
            }
        }
    }
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
