#include "servicetask.h"

#include <iostream>

#include "sleep.h"
#include "taskmanager.h"

namespace attic { 

ServiceTask::ServiceTask(TaskManager* tm,
                         const TaskContext& context)
                         :
                         Task(context, Task::SERVICE) {
    task_manager_ = tm;
    task_dispatch_ = NULL;
}
 
ServiceTask::~ServiceTask() {}


void ServiceTask::OnStart() {
    event::EventSystem::GetInstance()->Initialize();
    if(task_manager_) {
        task_dispatch_ = new TaskDispatch(task_manager_->file_manager(),
                                          task_manager_->credentials_manager(),
                                          task_manager_->access_token(),
                                          task_manager_->entity(),
                                          task_manager_->temp_directory(),
                                          task_manager_->working_directory(),
                                          task_manager_->config_directory());

    }

}

void ServiceTask::OnPaused() {}
void ServiceTask::OnFinished() {
    event::EventSystem::GetInstance()->Shutdown();
    if(task_dispatch_) {
        delete task_dispatch_;
        task_dispatch_ = NULL;
    }
}

void ServiceTask::RunTask() {
    event::EventSystem::GetInstance()->ProcessEvents();

    if(task_dispatch_) {
        task_dispatch_->Process(task_manager_);
        task_dispatch_->Dispatch();
    }
}

} //namespace
