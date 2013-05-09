#include "servicemanager.h"

#include <iostream>

#include "taskcontext.h"
#include "taskarbiter.h"
#include "errorcodes.h"
#include "servicetask.h"

namespace attic { 

ServiceManager::ServiceManager() {
    task_manager_ = NULL;
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
    TaskContext tc;
    // Create a Service Task
    Task* task = new ServiceTask(task_manager_, tc);
    // Spin it off, Arbiter takes ownership of task, don't worry about delete
    status = TaskArbiter::GetInstance()->SpinOffTask(task);

    return status;
}


}//namespace
