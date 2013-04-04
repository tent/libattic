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
#include "postfilemetadatastrategy.h"

namespace attic { 

DeleteTask::DeleteTask(FileManager* pFm, 
                       CredentialsManager* pCm,
                       const AccessToken& at,
                       const Entity& entity,
                       const std::string& filepath,
                       const std::string& tempdir, 
                       const std::string& workingdir,
                       const std::string& configdir,
                       TaskDelegate* callbackDelegate)
                       :
                       TentTask(Task::DELETE,
                                pFm,
                                pCm,
                                at,
                                entity,
                                filepath,
                                tempdir,
                                workingdir,
                                configdir,
                                callbackDelegate)
{
}

DeleteTask::~DeleteTask(){}

void DeleteTask::RunTask() {
    // Run the task
    int status = ret::A_OK;

    std::string filepath = TentTask::filepath();
    if(fs::CheckFilepathExists(filepath)) { 
        std::string post_path = TentTask::GetPostPath(); 

        SoftDeleteStrategy sds;
        PostFileMetadataStrategy pmds;

        HttpStrategyContext softdeletectx(GetFileManager(), 
                                          GetCredentialsManager());

        softdeletectx.SetConfigValue("post_path", post_path);
        softdeletectx.SetConfigValue("filepath", filepath);

        softdeletectx.PushBack(&sds);
        softdeletectx.PushBack(&pmds);

        status = softdeletectx.ExecuteAll();
    }
    
    // Callback
    Callback(status, "");
    SetFinishedState();
}

}//namespace
