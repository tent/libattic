#include "pushtask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "event.h"
#include "taskdelegate.h"
#include "filehandler.h"

#include "postfilestrategy.h"
#include "postfolderstrategy.h"
#include "postmetastrategy.h"

namespace attic {

PushTask::PushTask(FileManager* pFm, 
                   CredentialsManager* pCm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context)
                   :
                   TentTask(Task::PUSH,
                            pFm,
                            pCm,
                            at,
                            entity,
                            context)
{
}

PushTask::~PushTask() {}

void PushTask::OnPaused() { 
    if(!file_manager()->IsFileLocked(filepath())){
        SetRunningState();
    }
}

void PushTask::RunTask() {
    // Run the task
    std::string filepath = TentTask::filepath();
    int status = ret::A_FAIL_PATH_DOESNT_EXIST;
    std::cout<<" checking filepath exists : " << filepath << std::endl;
    if(fs::CheckFilepathExists(filepath)) {
        if(!file_manager()->IsFileLocked(filepath)) {
            file_manager()->LockFile(filepath);
            std::cout<<" PUSH NOTIFY ON PATH : " << filepath << std::endl;
            event::RaiseEvent(event::Event::PUSH, event::Event::START, filepath, NULL);
            status = PushFile(filepath);
            event::RaiseEvent(event::Event::PUSH, event::Event::DONE, filepath, NULL);
            file_manager()->UnlockFile(filepath);
        }
        else {
            std::string error = " File is locked by other task, finishing task\n";
            error += " file : " + filepath + "\n";
            log::LogString("333MA!941", error);
            status = ret::A_FAIL_FILE_IN_USE;
        }
    }
    else {
        std::cout<<" FILEPATH DOES NOT EXIST : " << filepath << std::endl;
    }

    Callback(status, filepath);
    SetFinishedState();
}

// Note* path should not be relative, let the filemanager take care of
// all the canonical to relative path conversions
int PushTask::PushFile(const std::string& filepath) {
    int status = ret::A_OK;
    if(fs::CheckFilepathExists(filepath)) {
        if(!file_manager()) std::cout<<" Invalid File Manager " << std::endl;
        if(!credentials_manager()) std::cout<<" Invalid Cred Manager " << std::endl;

        HttpStrategyContext pushcontext(file_manager(), credentials_manager());

        std::string post_path = GetPostPath();
        std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
        std::string entity = TentTask::entity().entity();

        pushcontext.SetConfigValue("post_path", post_path);
        pushcontext.SetConfigValue("posts_feed", posts_feed);
        pushcontext.SetConfigValue("filepath", filepath);
        pushcontext.SetConfigValue("entity", entity);

        PostFolderStrategy pfs;             // Check (and create) if directory posts exist
        PostMetaStrategy pmetas;            // Create meta strategy
        //PostFileStrategy ps;              // Chunk and upload
        // push back post folder strategy
        pushcontext.PushBack(&pfs);
        pushcontext.PushBack(&pmetas);
        //pushcontext.PushBack(&ps); 
        status = pushcontext.ExecuteAll();
        if(status == ret::A_OK) {
            std::cout<<" pushing file ... " << filepath << std::endl;
            FileHandler fh(file_manager());
            FileInfo fi;
            fh.RetrieveFileInfo(filepath, fi);
            std::cout<<" post id : " << fi.post_id() << std::endl;
            if(!fi.post_id().empty()) {
                // raise event to a start upload pipeline
                std::cout<<" raising event " << std::endl;
                event::RaiseEvent(event::Event::REQUEST_UPLOAD_FILE, fi.post_id(), NULL);
            }
            else {
                status = ret::A_FAIL_INVALID_POST_ID;
            }
        }
    }
    else {
        status = ret::A_FAIL_OPEN_FILE;
    }

    std::cout<<" end push task status : " << status << std::endl;

    return status;
}

}//namespace
