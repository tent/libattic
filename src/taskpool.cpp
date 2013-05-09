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
    TaskMap::iterator itr = task_map_.begin();
    for(;itr != task_map_.end(); itr++) {
        if(itr->second.size()) {
            std::cout<< "Queue Type : " << itr->first << std::endl;
            std::cout<< "Queue Size : " << itr->second.size() << std::endl;
            TaskQueue::iterator q_itr = itr->second.begin();
            for(;q_itr != itr->second.end();q_itr++) {
                Task* task = *q_itr; 
                (*q_itr) = NULL;
                if(task) {
                    delete task;
                    task = NULL;
                }
            }
        }
        itr->second.clear();
    }
    task_map_.clear();
}

void TaskPool::PushBack(Task* task) {
    std::cout<<" pushing back task type : " << task->type() << std::endl;
    Lock();
    if(task) { 
        task_map_[task->type()].push_back(task);
        total_task_count_++;
    }
    Unlock();
}

Task* TaskPool::Remove(Task* task) {
    if(task) {
        Lock();
        // Find
        TaskQueue::iterator itr = FindTask(task, task->type());
        if(itr != task_map_[task->type()].end()) {
            // Remove
            task = *itr;                
            *itr = NULL;
        }
        Unlock();
    }
    return task;
}

Task* TaskPool::RequestTask(const Task::TaskType type) {
    Task* t = NULL;
    Lock();
    if(task_map_[type].size()) {
        t = task_map_[type].front();
        task_map_[type].pop_front();
        active_task_count_++;
    }
    Unlock();
    return t;
}

Task* TaskPool::RequestNextAvailableTask() {
    Task* t = NULL;
    Lock();
    TaskMap::iterator itr = task_map_.begin();
    for(;itr!= task_map_.end();itr++) {
        if(itr->second.size() > 0) {
            std::cout<<" \t task type : " << itr->first << std::endl;
            std::cout<<" \t queue size (before) : " << itr->second.size() << std::endl;
            t = itr->second.front();
            itr->second.pop_front();
            active_task_count_++;
            std::cout<<" \t queue size (after) : " << itr->second.size() << std::endl;
            break;
        }
    }
    Unlock();
    return t;
}

void TaskPool::ReclaimTask(Task* task) {
    std::cout<<" RECLAMING TASK : " << task << std::endl;
    if(task) {
        delete task;
        task = NULL;
        reclaimed_task_count_++;
    }
}

TaskPool::TaskQueue::iterator TaskPool::FindTask(Task* task, Task::TaskType type) {
    TaskQueue::iterator itr = task_map_[type].begin();
    if(task) {
        for(;itr != task_map_[type].end(); itr++) {
            if((*itr) == task)
                break;
        }
    }
    else
        itr = task_map_[type].end();

    return itr;
}

bool TaskPool::HasTaskOfType(Task::TaskType type) {
    if(task_map_[type].size() > 0)
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
