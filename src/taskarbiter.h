#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include "mutexclass.h"
#include "taskqueue.h"
#include "taskmanager.h"

namespace attic { 

class Task;
class ThreadPool;

class TaskArbiter : public MutexClass{
    TaskArbiter();
    TaskArbiter(const TaskArbiter& rhs) {}
    TaskArbiter operator=(const TaskArbiter& rhs) { return *this; }
public:
    ~TaskArbiter();

    int Initialize(unsigned int poolSize);
    int Shutdown();

    static TaskArbiter* GetInstance();

    Task* RequestTask();
    Task* RequestTask(Task::TaskType type);
    void ReclaimTask(Task* task);
    void PushBackTask(Task* pTask);
    unsigned int ActiveTaskCount();

    int CreateAndSpinOffTask(const TaskContext& tc);
    int SpinOffTask(Task* pTask);

    TaskManager* task_manager() { return task_manager_; }
    void set_task_manager(TaskManager* task_manager) { task_manager_ = task_manager; }
private:
    static bool            initialized_;
    static TaskArbiter*    instance_;
    TaskManager*           task_manager_;

    ThreadPool*            thread_pool_;
    TaskPool               task_pool_;
};

} //namespace
#endif

