#include "servicetask.h"

#include <iostream>
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
    std::cout<<" SERVICE TASK ON START " << std::endl;
    event::EventSystem::GetInstance()->Initialize();

}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {
    std::cout<<" SERVICE TASK ON FINISHED " << std::endl;
    event::EventSystem::GetInstance()->Shutdown();
}

void ServiceTask::RunTask() {
    std::cout<<" RUNNING SERVICE TASK " << std::endl;
    event::EventSystem::GetInstance()->ProcessEvents();

}

} //namespace
