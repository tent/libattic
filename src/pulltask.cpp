#include "pulltask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "utils.h"
#include "event.h"
#include "netlib.h"
#include "taskdelegate.h"

#include "getfilestrategy.h"

namespace attic { 

PullTask::PullTask(FileManager* fm, 
                   CredentialsManager* cm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context)
                   :
                   TentTask(Task::PULL,
                            fm,
                            cm,
                            at,
                            entity,
                            context)
{
}

PullTask::~PullTask() {}

void PullTask::OnPaused() { 
    std::cout<<" PULL TASK IS PAUSED " << std::endl;
    if(!file_manager()->IsFileLocked(filepath())){
        SetRunningState();
    }
}

void PullTask::RunTask() {
    std::string filepath = TentTask::filepath();
    int status = ret::A_OK;
    if(!file_manager()->IsFileLocked(filepath)) {
        file_manager()->LockFile(filepath);
        event::RaiseEvent(event::Event::PULL, event::Event::START, filepath, NULL);
        status = PullFile(filepath);
        event::RaiseEvent(event::Event::PULL, event::Event::DONE, filepath, NULL);
        file_manager()->UnlockFile(filepath);
    }
    else {
        std::string error = " File is locked by other task, finishing task\n";
        error += " file : " + filepath + "\n";
        log::LogString("1#819kmapAm", error);
        status = ret::A_FAIL_FILE_IN_USE;
    }

    if(status != ret::A_OK) {
        std::ostringstream error;
        error << " Pull Task failed for : " << filepath << std::endl;
        log::LogString("pull_1841", error.str());
    }

    Callback(status, filepath);
    SetFinishedState();
}

int PullTask::PullFile(const std::string& filepath) {
    int status = ret::A_OK;

    std::string post_path = GetPostPath();
    std::string post_attachment;
    utils::FindAndReplace(entity()->GetPreferredServer().attachment(),
                          "{entity}",
                          entity()->entity(),
                          post_attachment);
    Response resp;

    GetFileStrategy gfs;
    HttpStrategyContext pullcontext(file_manager(), 
                                    credentials_manager());

    std::string posts_feed = entity()->GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity()->entity();

    pullcontext.SetConfigValue("post_path",post_path);
    pullcontext.SetConfigValue("posts_feed", posts_feed);
    pullcontext.SetConfigValue("post_attachment", post_attachment);
    pullcontext.SetConfigValue("filepath", filepath);
    pullcontext.SetConfigValue("entity", entity);

    pullcontext.PushBack(&gfs);

    status = pullcontext.ExecuteAll();

    return status; 
}

}//namespace

