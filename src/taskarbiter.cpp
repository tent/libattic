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

    CentralTaskQueue::GetInstance()->Shutdown();
    return status;
}

TaskArbiter* TaskArbiter::GetInstance() {
    if(!instance_)
        instance_ = new TaskArbiter();
    return instance_;
}

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
int TaskArbiter::SpinOffTask(Task* pTask) {
    int status = ret::A_OK;

    if(initialized_) { 
        CentralTaskQueue::GetInstance()->SyncPushBack(pTask);
    }
    else
        status = ret::A_FAIL_SUBSYSTEM_NOT_INITIALIZED;
    return status;
}

}//namespace
