#include "uploadtask.h"

namespace attic { 

UploadTask::UploadTask(FileManager* fm,
                       CredentialsManager* cm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context) 
                       :
                       TentTask(Task::UPLOADFILE,
                                fm,
                                cm,
                                at,
                                entity,
                                context)
{
}

UploadTask::~UploadTask() {}

void UploadTask::RunTask() {
    int status = ret::A_OK;
    std::string post_id;
    context_.get_value("post_id", post_id);
    if(!post_id.empty()) { 
        status = ProcessFile(post_id);
    }
    else {
        status = ret::A_FAIL_INVALID_POST_ID;
    }

    Callback(status, post_id);
    SetFinishedState();
}

int UploadTask::ProcessFile(const std::string& post_id) {
    int status = ret::A_OK;

    // Retrieve File Info

    return status;
}

} // namespace

