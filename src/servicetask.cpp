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
    //task_dispatch_ = NULL;
}
 
ServiceTask::~ServiceTask() {}


void ServiceTask::OnStart() {
    std::cout<<" SERVICE TASK STARTED " << std::endl;
    event::EventSystem::instance()->Initialize();
    /*
    if(task_manager_) {
        task_dispatch_ = new TaskDispatch(file_manager(),
                                          credentials_manager(),
                                          access_token(),
                                          entity(),
                                          temp_directory(),
                                          working_directory(),
                                          config_directory());

    }
    */

}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {
    std::cout<<" SERVICE TASK FINISHED ... " << std::endl;
    event::EventSystem::instance()->Shutdown();
    /*
    if(task_dispatch_) {
        delete task_dispatch_;
        task_dispatch_ = NULL;
    }
    */
}

void ServiceTask::RunTask() {
    event::EventSystem::instance()->ProcessEvents();
    /*
    if(task_dispatch_) {
        task_dispatch_->Process(task_manager_);
        task_dispatch_->Dispatch();
    }
    */
}

} //namespace
