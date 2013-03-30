#include "servicetask.h"

namespace attic { 

ServiceTask::ServiceTask(FileManager* pFm,
                         CredentialsManager* pCm,
                         const AccessToken& at,
                         const Entity& entity,
                         const std::string& filepath,
                         const std::string& tempdir,
                         const std::string& workingdir,
                         const std::string& configdir,
                         TaskDelegate* callbackDelegate) 
                         :
                         TentTask(Task::SERVICE,
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
 
ServiceTask::~ServiceTask() {}

void ServiceTask::OnStart() {

}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {}

void ServiceTask::RunTask() {}

} //namespace
