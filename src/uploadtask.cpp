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

}


} // namespace

