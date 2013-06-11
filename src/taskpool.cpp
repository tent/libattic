#include "taskpool.h"

namespace attic {

TaskPool::TaskPool() {
    total_task_count_ = 0;
    active_task_count_ = 0;
    reclaimed_task_count_ = 0;
}

TaskPool::~TaskPool() {
   ClearPool(); 
}

void TaskPool::ClearPool() {
    ContextMap::iterator itr = context_map_.begin();
    for(;itr != context_map_.end(); itr++)
        itr->second.clear();
    context_map_.clear();
}

void TaskPool::PushBack(const TaskContext& tc) {
    Lock();
    std::cout<<" Pushing back task context of type : " << tc.type() << std::endl;
    context_map_[tc.type()].push_back(tc);
    total_task_count_++;
    Unlock();
}


bool TaskPool::RequestTaskContext(const Task::TaskType type, TaskContext& out) {
    bool val = false;
    Lock();
    if(context_map_[type].size()) {
        out = context_map_[type].front();
        context_map_[type].pop_front();
        active_task_count_++;
        val = true;
    }
    Unlock();
    return val;
}

bool TaskPool::RequestNextAvailableTaskContext(TaskContext& out) {
    bool val = false;
    Lock();
    ContextMap::iterator itr = context_map_.begin();
    for(;itr!= context_map_.end();itr++) {
        if(itr->second.size() > 0) {
            std::cout<<" \t task type : " << itr->first << std::endl;
            std::cout<<" \t queue size (before) : " << itr->second.size() << std::endl;
            out = itr->second.front();
            itr->second.pop_front();
            active_task_count_++;
            std::cout<<" \t queue size (after) : " << itr->second.size() << std::endl;
            val = true;
            break;
        }
    }
    Unlock();
    return val;
}

bool TaskPool::HasTaskContextOfType(Task::TaskType type) {
    if(context_map_[type].size() > 0)
        return true;
    return false;
}

unsigned int TaskPool::ActiveTaskCount() {
    Lock();
    unsigned int count = active_task_count_;
    Unlock();
    return count;
}

}//namespace
