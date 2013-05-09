#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

namespace attic { 

TaskArbiter* TaskArbiter::instance_ = 0;
bool TaskArbiter::initialized_ = false;

TaskArbiter::TaskArbiter() {
    thread_pool_ = new ThreadPool();
    task_manager_ = NULL;
}

TaskArbiter::~TaskArbiter() {
    if(thread_pool_) {
        delete thread_pool_;
        thread_pool_ = NULL;
    }
}

int TaskArbiter::Initialize(unsigned int poolSize) {
    int status = ret::A_OK;
    Lock();
    if(thread_pool_) {
        thread_pool_->Initialize();
        thread_pool_->ExtendPool(poolSize);
        initialized_ = true;
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }
    Unlock();
    return status; 
}

int TaskArbiter::Shutdown() {
    int status = ret::A_OK;

    std::cout<<" 1 " << std::endl;
    Lock();
    if(thread_pool_)
        status = thread_pool_->Shutdown();
    Unlock();

    task_pool_.ClearPool();
    std::cout<<" 1 " << std::endl;
    // DO THIS LAST ... if you have any mutex derived member varibales it will delete them...
    // and may cause DEADLOCK ...DUH
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }

    std::cout<<" 1 " << std::endl;

    std::cout<<" 1 " << std::endl;
    return status;
}

TaskArbiter* TaskArbiter::GetInstance() {
    if(!instance_)
        instance_ = new TaskArbiter();
    return instance_;
}

int TaskArbiter::CreateAndSpinOffTask(const TaskContext& tc) {
    int status = ret::A_OK; 
    Task* t = task_manager_->GetTentTask(tc);
    status = SpinOffTask(t);
    return status;
}       


// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
int TaskArbiter::SpinOffTask(Task* pTask) {
    int status = ret::A_OK;
    if(initialized_)
        task_pool_.PushBack(pTask);
    else
        status = ret::A_FAIL_SUBSYSTEM_NOT_INITIALIZED;
    return status;
}

void TaskArbiter::PushBackTask(Task* task) {
    task_pool_.PushBack(task);
}

Task* TaskArbiter::RequestTask() {
    return task_pool_.RequestNextAvailableTask();
}

Task* TaskArbiter::RequestTask(Task::TaskType type) {
    return task_pool_.RequestTask(type);
}

void TaskArbiter::ReclaimTask(Task* task) {
    task_pool_.ReclaimTask(task);
}

unsigned int TaskArbiter::ActiveTaskCount() {
    return task_pool_.ActiveTaskCount();
}

}//namespace

