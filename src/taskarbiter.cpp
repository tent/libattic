#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

namespace attic { 

TaskArbiter* TaskArbiter::instance_ = 0;
bool TaskArbiter::initialized_ = false;

TaskArbiter::TaskArbiter() {
    task_manager_ = NULL;
}

TaskArbiter::~TaskArbiter() {}

int TaskArbiter::Initialize(TaskManager* tm) {
    int status = ret::A_OK;
    task_manager_ = tm;
    initialized_ = true;
    return status; 
}

int TaskArbiter::Shutdown() {
    int status = ret::A_OK;

    task_pool_.ClearPool();
    task_manager_ = NULL;
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
    bool ret = task_pool_.RequestNextAvailableTaskContext(out);
    if(ret)
        RemoveFromFilter(out);
    return ret;
}

bool TaskArbiter::RequestTaskContext(Task::TaskType type, TaskContext& out) {
    bool ret = task_pool_.RequestTaskContext(type, out);
    if(ret) {
        RemoveFromFilter(out);
    }
    return ret;
}

unsigned int TaskArbiter::ActiveTaskCount() {
    return task_pool_.ActiveTaskCount();
}

void TaskArbiter::RetrieveTasks() {
    if(task_manager_) {
        TaskContext::ContextQueue cq;
        task_manager_->RetrieveContextQueue(cq);
        TaskContext::ContextQueue::iterator itr = cq.begin();
        for(;itr!=cq.end();itr++) {
            if(!FilterTask(*itr)) { 
                task_pool_.PushBack(*itr);
            }
        }
        if(cq.size()) {
            std::cout<< "pushing back " << cq.size() << " tasks " << std::endl;
        }
    }
}

bool TaskArbiter::FilterTask(const TaskContext& tc) {
    bool ret = false;
    std::cout<<" filter task type : " << tc.type() << std::endl;
    if(tc.type() == Task::PUSH || tc.type() == Task::PULL) {
        std::string key;
        if(tc.get_value("filepath", key)) {
            if(CheckDuplicateTask(tc.type(), key)) {
                std::cout<<" task duplicate ... filtered key : " << key << std::endl;
                ret = true;
            }
            else {
                std::cout<<" not a dup, filtering key : " << key << std::endl;
                filter_mtx_.Lock();
                task_filter_[tc.type()][key] = true;
                filter_mtx_.Unlock();
            }
        }
    }
    return ret;
}

void TaskArbiter::RemoveFromFilter(const TaskContext& tc) {
    std::cout<<" remove from filter type : " << tc.type() << std::endl;
    if(tc.type() == Task::PUSH || tc.type() == Task::PULL) {
        std::string key;
        if(tc.get_value("filepath", key)) {
            if(CheckDuplicateTask(tc.type(), key)) {
                // Remove task
                std::cout<<" removing filter for : " << key << std::endl;
                filter_mtx_.Lock();
                task_filter_[tc.type()].erase(key);
                filter_mtx_.Unlock();
            }
        }
    }
}


bool TaskArbiter::CheckDuplicateTask(int task_type, const std::string& key) {
    bool ret = false;
    filter_mtx_.Lock();
    if(task_filter_[task_type].find(key) != task_filter_[task_type].end())
        ret = true;
    filter_mtx_.Unlock();
    return ret;
}

}//namespace

