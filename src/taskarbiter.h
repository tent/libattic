#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include "mutexclass.h"
#include "taskqueue.h"
#include "taskmanager.h"
#include "taskcontext.h"

namespace attic { 

class Task;
class ThreadPool;

class TaskArbiter : public MutexClass{
    TaskArbiter();
    TaskArbiter(const TaskArbiter& rhs) {}
    TaskArbiter operator=(const TaskArbiter& rhs) { return *this; }
public:
    ~TaskArbiter();

    int Initialize(TaskManager* tm);
    int Shutdown();

    static TaskArbiter* GetInstance();

    void PushBackTask(const TaskContext& tc);
    bool RequestTaskContext(TaskContext& out);
    bool RequestTaskContext(Task::TaskType type, TaskContext& out);

    unsigned int ActiveTaskCount();

    void RetrieveTasks();

    TaskManager* task_manager() { return task_manager_; }
    void set_task_manager(TaskManager* task_manager) { task_manager_ = task_manager; }
private:
    static bool            initialized_;
    static TaskArbiter*    instance_;
    TaskManager*           task_manager_;

    TaskPool               task_pool_;
};

} //namespace
#endif

