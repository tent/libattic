#include "servicetask.h"

namespace attic { 

ServiceTask::ServiceTask(FileManager* pFm,
                         CredentialsManager* pCm,
                         const AccessToken& at,
                         const Entity& entity,
                         const TaskContext& context,
                         TaskDelegate* callbackDelegate) 
                         :
                         TentTask(Task::SERVICE,
                                  pFm,
                                  pCm,
                                  at,
                                  entity,
                                  context,
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
