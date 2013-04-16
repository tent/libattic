#include "taskpool.h"

namespace attic {

TaskPool::TaskPool() {}

TaskPool::~TaskPool() {
    TaskMap::iterator itr = task_map_.begin();

    for(;itr != task_map_.end(); itr++) {
        for(;itr->second.size(); ) {
            Task* task = itr->second.front();
            delete task;
            task = NULL;
            itr->second.pop_front();
        }
    }
}

void TaskPool::PushBack(Task* task) {
    Lock();
    if(task)
        task_map_[task->type()].push_back(task);
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

}//namespace
