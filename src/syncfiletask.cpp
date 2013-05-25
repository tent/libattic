#include "syncfiletask.h"

#include <vector>

#include "response.h"
#include "urlparams.h"
#include "netlib.h"
#include "jsonserializable.h"
#include "entity.h"
#include "response.h"
#include "event.h"
#include "filesystem.h"
#include "taskdelegate.h"
#include "logutils.h"
#include "configmanager.h"

#include "renamehandler.h"
#include "posthandler.h"
#include "filehandler.h"
#include "folderhandler.h"

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
    std::string posturl;
    utils::FindAndReplace(GetPostPath(), "{post}", post_id_, posturl);
    PostHandler<FilePost> ph(access_token());
    Response response;
    status = ph.Get(posturl, NULL, out, response);
    return status;
}

int SyncFileTask::ProcessFileInfo(FilePost& p) {
    int status = ret::A_OK;
    FileManager* fm = file_manager();
    if(!fm) return ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
    std::string filepath = p.relative_path();
    FileHandler fh(file_manager());
    // Check if any aliases exist, and fix
    RenameHandler rh(file_manager());
    if(!rh.CheckForRename(p)) {
        FileInfo fi;
        fh.DeserializeIntoFileInfo(p, fi);
        // Check if file is in manifest
        //int version = p.GetVersion();

        // Get Local file info
        FileInfo* pLocal_fi = fm->GetFileInfo(filepath);

        bool bPull = false;
        if(pLocal_fi) {
            std::string canonical_path;
            fm->GetCanonicalFilepath(filepath, canonical_path);
            // check if file exists, locally
            if(pLocal_fi->deleted()) {
                bPull = false;
            }
            else if(!fs::CheckFilepathExists(canonical_path)) {
                bPull = true;
            }

            if(pLocal_fi->post_version() != p.version()->id())
                bPull = true;
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
    return status;
}

void SyncFileTask::ValidateFileInfo(FileInfo& incoming, FileInfo& local) {
    // TODO :: update local fileinfo
    ValidateFilepath(local);
}

// During processing, a folder post could have changed, validate that the filepath of this file
// and update if necessary
void SyncFileTask::ValidateFilepath(FileInfo& fi) {
    FolderHandler fh(file_manager());
    Folder folder;
    if(fh.GetFolderById(fi.folder_post_id(), folder)) {
        std::string folderpath = folder.folderpath();
        utils::CheckUrlAndAppendTrailingSlash(folderpath);

        std::string filepath = fi.filepath();
        size_t pos = filepath.rfind("/");
        if(pos != std::string::npos) {
            filepath = filepath.substr(0, pos+1);
            std::cout<<" validating filepath ... " << std::endl;
            std::cout<<" \t local (file)folderpath : " << filepath << std::endl;
            std::cout<<" \t folderpath : " << folderpath << std::endl;
            if(filepath != folderpath) {
                std::cout<<" LOCAL CACHE FILEPATH IN CONFLICT " << std::endl;
                // Update filepath
                std::string new_filepath = folderpath + fi.filename();
                // Update local cache
                FileHandler fileh(file_manager());
                if(fileh.UpdateFilepath(fi.filepath(), new_filepath)) {
                    std::cout<<" updating filepath ... " << std::endl;
                    fi.set_filepath(new_filepath);
                }
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
