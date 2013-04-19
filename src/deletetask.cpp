#include "deletetask.h"

#include "filemanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "fileinfo.h"
#include "postutils.h"
#include "netlib.h"
#include "taskdelegate.h"
#include "filesystem.h"

#include "softdeletestrategy.h"

namespace attic { 

DeleteTask::DeleteTask(FileManager* pFm, 
                       CredentialsManager* pCm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context,
                       TaskDelegate* callbackDelegate)
                       :
                       TentTask(Task::DELETE,
                                pFm,
                                pCm,
                                at,
                                entity,
                                context,
                                callbackDelegate)
{
}

DeleteTask::~DeleteTask(){}

void DeleteTask::RunTask() {
    // Run the task
    int status = ret::A_OK;

    std::string filepath = TentTask::filepath();
    std::string post_path = TentTask::GetPostPath(); 
    std::string entity = TentTask::entity().entity();

    SoftDeleteStrategy sds;

    HttpStrategyContext softdeletectx(file_manager(), 
                                      credentials_manager());

    softdeletectx.SetConfigValue("post_path", post_path);
    softdeletectx.SetConfigValue("filepath", filepath);
    softdeletectx.SetConfigValue("entity", entity);

    softdeletectx.PushBack(&sds);

    status = softdeletectx.ExecuteAll();
    
    // Callback
    Callback(status, "");
    SetFinishedState();
}

}//namespace
