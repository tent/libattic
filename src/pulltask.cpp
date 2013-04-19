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

PullTask::PullTask(FileManager* pFm, 
                   CredentialsManager* pCm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context,
                   TaskDelegate* callbackDelegate)
                   :
                   TentTask( Task::PULL,
                             pFm,
                             pCm,
                             at,
                             entity,
                             context,
                             callbackDelegate)
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

    if(file_manager()->IsFileLocked(filepath))
        SetPausedState();
    else {
        std::cout<<" RUNNING PULL TASK " << std::endl;
        file_manager()->LockFile(filepath);
        event::RaiseEvent(event::Event::PULL, event::Event::START, filepath, NULL);
        int status = PullFile(filepath);
        event::RaiseEvent(event::Event::PULL, event::Event::DONE, filepath, NULL);
        file_manager()->UnlockFile(filepath);

        std::cout<<" PULL TASK FINISHED : " << status << std::endl;
        Callback(status, filepath);
        SetFinishedState();
    }
}

int PullTask::PullFile(const std::string& filepath) {
    int status = ret::A_OK;

    std::string post_path = GetPostPath();
    std::string post_attachment;
    utils::FindAndReplace(TentTask::entity().GetPreferredServer().attachment(),
                          "{entity}",
                          TentTask::entity().entity(),
                          post_attachment);
    Response resp;

    GetFileStrategy gfs;
    HttpStrategyContext pullcontext(file_manager(), 
                                    credentials_manager());

    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

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

