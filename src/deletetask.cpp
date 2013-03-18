#include "deletetask.h"

#include "filemanager.h"

#include "errorcodes.h"
#include "utils.h"
#include "fileinfo.h"
#include "postutils.h"
#include "netlib.h"
#include "taskdelegate.h"

#include "softdeletestrategy.h"
#include "postfilemetadatastrategy.h"

DeleteTask::DeleteTask( TentApp* pApp, 
                        FileManager* pFm, 
                        CredentialsManager* pCm,
                        TaskArbiter* pTa,
                        TaskFactory* pTf,
                        const AccessToken& at,
                        const Entity& entity,
                        const std::string& filepath,
                        const std::string& tempdir, 
                        const std::string& workingdir,
                        const std::string& configdir,
                        TaskDelegate* callbackDelegate)
                        :
                        TentTask( Task::DELETE,
                                  pApp,
                                  pFm,
                                  pCm,
                                  pTa,
                                  pTf,
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

    std::string filepath, apiroot;
    GetFilepath(filepath);
    GetApiRoot(apiroot);

    SoftDeleteStrategy sds;
    PostFileMetadataStrategy pmds;

    HttpStrategyContext softdeletectx(GetFileManager(), 
                                      GetCredentialsManager(), 
                                      apiroot, 
                                      filepath);

    softdeletectx.PushBack(&sds);
    softdeletectx.PushBack(&pmds);

    status = softdeletectx.ExecuteAll();
    
    // Callback
    Callback(status, "");
    SetFinishedState();
}

