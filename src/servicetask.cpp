#include "servicetask.h"

#include "sleep.h"

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
    event::EventSystem::GetInstance()->Initialize();

}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {
    event::EventSystem::GetInstance()->Shutdown();
}

void ServiceTask::RunTask() {
    event::EventSystem::GetInstance()->ProcessEvents();

}

} //namespace
