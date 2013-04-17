#include "renametask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "utils.h"
#include "event.h"
#include "netlib.h"
#include "taskdelegate.h"

namespace attic { 

RenameTask::RenameTask(FileManager* pFm, 
                       CredentialsManager* pCm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context,
                       TaskDelegate* callbackDelegate) 
                       :
                       TentTask(Task::RENAME,
                                pFm,
                                pCm,
                                at,
                                entity,
                                context,
                                callbackDelegate)
{
}

RenameTask::~RenameTask() {}

void RenameTask::RunTask() {
    std::cout<<" Rename task run ... " << std::endl;
}

} //namespace
