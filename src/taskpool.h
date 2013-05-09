#ifndef TASKPOOL_H_
#define TASKPOOL_H_
#pragma once

#include <deque>
#include <map>

#include "mutexclass.h"
#include "task.h"

namespace attic {

class TaskPool : public MutexClass {
    typedef std::deque<Task*> TaskQueue;
    typedef std::map<Task::TaskType, TaskQueue> TaskMap;

    TaskQueue::iterator FindTask(Task* task, Task::TaskType type);

public:
    TaskPool();
    ~TaskPool();

    void ClearPool();

    void PushBack(Task* task);
    Task* Remove(Task* task);

    bool HasTaskOfType(const Task::TaskType type);
    Task* RequestTask(const Task::TaskType type);
    Task* RequestNextAvailableTask();
    void ReclaimTask(Task* task);

    unsigned int ActiveTaskCount();
        
    TaskQueue* operator[](const Task::TaskType type) {
        return &task_map_[type];
    }
private:
    unsigned int total_task_count_;
    unsigned int active_task_count_;
    unsigned int reclaimed_task_count_;
    TaskMap task_map_;
};

}//namespace
#endif

