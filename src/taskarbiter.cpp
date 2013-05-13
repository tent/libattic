#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

namespace attic { 

TaskArbiter* TaskArbiter::instance_ = 0;
bool TaskArbiter::initialized_ = false;

TaskArbiter::TaskArbiter() {
    //task_manager_ = NULL;
}

TaskArbiter::~TaskArbiter() {}

int TaskArbiter::Initialize(unsigned int poolSize) {
    int status = ret::A_OK;
    initialized_ = true;
    return status; 
}

int TaskArbiter::Shutdown() {
    int status = ret::A_OK;

    task_pool_.ClearPool();
    std::cout<<" 1 " << std::endl;
    // DO THIS LAST ... if you have any mutex derived member varibales it will delete them...
    // and may cause DEADLOCK ...DUH
    if(instance_) {
        delete instance_;
        instance_ = NULL;
    }
    return status;
}

TaskArbiter* TaskArbiter::GetInstance() {
    if(!instance_)
        instance_ = new TaskArbiter();
    return instance_;
}

void TaskArbiter::PushBackTask(const TaskContext& tc) {
    task_pool_.PushBack(tc);
}

bool TaskArbiter::RequestTaskContext(TaskContext& out) {
    return task_pool_.RequestNextAvailableTaskContext(out);
}

bool TaskArbiter::RequestTaskContext(Task::TaskType type, TaskContext& out) {
    return task_pool_.RequestTaskContext(type, out);
}

unsigned int TaskArbiter::ActiveTaskCount() {
    return task_pool_.ActiveTaskCount();
}

}//namespace

