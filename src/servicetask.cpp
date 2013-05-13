#include "servicetask.h"

#include <iostream>
#include "sleep.h"
#include "taskmanager.h"

namespace attic { 

ServiceTask::ServiceTask(FileManager* pFm, 
                         CredentialsManager* pCm,
                         const AccessToken& at,
                         const Entity& entity,
                         const TaskContext& context)
                         :
                         TentTask(Task::SERVICE,
                                  pFm,
                                  pCm,
                                  at,
                                  entity,
                                  context) {
}
 
ServiceTask::~ServiceTask() {}


void ServiceTask::OnStart() {
    std::cout<<" SERVICE TASK STARTED " << std::endl;
    event::EventSystem::instance()->Initialize();
}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {
    std::cout<<" SERVICE TASK FINISHED ... " << std::endl;
    event::EventSystem::instance()->Shutdown();
}

void ServiceTask::RunTask() {
    event::EventSystem::instance()->ProcessEvents();
}

} //namespace
