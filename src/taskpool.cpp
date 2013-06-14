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
    context_map_[tc.type()].push_back(tc);
    total_task_count_++;
    Unlock();

    //std::cout<< stats() << std::endl;
}

std::string TaskPool::stats() {
    std::ostringstream s;
    Lock();
    s << " TaskPool Stats : " << std::endl;
    s << "\t Context Map size (total) : " << context_map_.size() << std::endl;
    ContextMap::iterator itr = context_map_.begin();
    for(;itr!=context_map_.end(); itr++) { 
        s << "\t\t task type : " << itr->first << " count : " << itr->second.size() << std::endl;
    }
    s << " TaskPool Stats end " << std::endl;
    Unlock();
    return s.str();
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
            out = itr->second.front();
            itr->second.pop_front();
            active_task_count_++;
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
