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

    TaskQueue::iterator FindTask(Task* pTask, Task::TaskType type);

public:
    TaskPool();
    ~TaskPool();

    void PushBack(Task* pTask);
    Task* Remove(Task* pTask);
        
    TaskQueue* operator[](const Task::TaskType type)
    {
        return &m_TaskMap[type];
    }
private:

    TaskMap m_TaskMap;
};

}//namespace
#endif

