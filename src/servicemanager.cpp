#include "servicemanager.h"

#include <iostream>

#include "taskcontext.h"
#include "taskarbiter.h"
#include "errorcodes.h"
#include "servicetask.h"

namespace attic { 

ServiceManager::ServiceManager(TaskManager* tm) {
    task_manager_ = tm;
}

ServiceManager::~ServiceManager() {

}

int ServiceManager::Initialize() {
    int status = ret::A_OK;
    status = StartupServiceThread();
    return status;
}

int ServiceManager::Shutdown() {
    int status = ret::A_OK;
    return status;
}

int ServiceManager::StartupServiceThread() {
    int status = ret::A_OK;
    std::cout<<" STARTING UP SERVICE THREAD " << std::endl;
    // Create a Service Task
    TaskContext tc = task_manager_->CreateServiceContext();

    TaskArbiter::GetInstance()->PushBackTask(tc);

    return status;
}


}//namespace
