#include "pulltask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "utils.h"
#include "event.h"
#include "netlib.h"
#include "taskdelegate.h"

#include "getfilestrategy.h"

PullTask::PullTask( TentApp* pApp, 
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
                    TentTask( Task::PULL,
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

PullTask::~PullTask() {}

void PullTask::RunTask() {
    std::string filepath;
    GetFilepath(filepath);

    event::RaiseEvent(event::Event::PULL, event::Event::START, filepath, NULL);
    int status = PullFile(filepath);
    event::RaiseEvent(event::Event::PULL, event::Event::DONE, filepath, NULL);

    Callback(status, filepath);
    SetFinishedState();
}

int PullTask::PullFile(const std::string& filepath) {
    int status = ret::A_OK;

    std::string apiroot;
    GetApiRoot(apiroot);
    Response resp;

    GetFileStrategy gfs;
    HttpStrategyContext pullcontext(GetFileManager(), 
                                    GetCredentialsManager());

    pullcontext.SetConfigValue("api_root", apiroot);
    pullcontext.SetConfigValue("filepath", filepath);

    pullcontext.PushBack(&gfs);

    status = pullcontext.ExecuteAll();

 
    return status; 
}

