#include "pushpublictask.h"

#include "filesystem.h"

namespace attic { 

PushPublicTask::PushPublicTask(FileManager* fm,
                               CredentialsManager* cm,
                               const AccessToken& at,
                               const Entity& entity,
                               const TaskContext& context)
                               :
                               TentTask(Task::PUSHPUBLIC,
                                        fm,
                                        cm,
                                        at,
                                        entity,
                                        context) 
{
}

PushPublicTask::~PushPublicTask() {}

void PushPublicTask::RunTask() {
    std::string filepath = TentTask::filepath();
    int status = ret::A_FAIL_PATH_DOESNT_EXIST;
    if(fs::CheckFilepathExists(filepath)) {
        status = PushFile(filepath);
    }
    Callback(status, filepath);
    SetFinishedState();
}

int PushPublicTask::PushFile(const std::string& filepath) {
    int status = ret::A_OK;
    // Create Download Post
    // Create public link
    // Call

    return status;
}



} // namespace

