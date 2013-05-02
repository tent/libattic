#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

namespace attic { 

TaskArbiter* TaskArbiter::instance_ = 0;
bool TaskArbiter::initialized_ = false;

TaskArbiter::TaskArbiter() {
    pool_ = new ThreadPool();
    task_manager_ = NULL;
}

TaskArbiter::~TaskArbiter() {
    if(pool_) {
        delete pool_;
        pool_ = NULL;
    }
}

int TaskArbiter::Initialize(unsigned int poolSize) {
    int status = ret::A_OK;
    Lock();
    if(pool_) {
        pool_->Initialize();
        pool_->ExtendPool(poolSize);
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

    Lock();
    if(pool_)
        status = pool_->Shutdown();
    Unlock();

    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }

    task_queue_.ClearTaskQueue();
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
        task_queue_.SyncPushBack(pTask);
    else
        status = ret::A_FAIL_SUBSYSTEM_NOT_INITIALIZED;
    return status;
}

void TaskArbiter::SyncPushBack(Task* task) {
    task_queue_.SyncPushBack(task);
}

Task* TaskArbiter::SyncPopFront() {
    return task_queue_.SyncPopFront();
}

void TaskArbiter::ReclaimTask(Task* task) {
    task_queue_.ReclaimTask(task);
}

unsigned int TaskArbiter::ActiveTaskCount() {
    return task_queue_.ActiveTaskCount();
}

}//namespace

