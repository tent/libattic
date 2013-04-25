#ifndef SERVICETASK_H_
#define SERVICETASK_H_
#pragma once

#include "task.h"
#include "taskdispatch.h"

namespace attic { 
// This task encapsulates general purpose systems that need dt updating
// Event system
// - Time sensative delete queues etc (future)
//
class TaskManager;
class ServiceTask : public Task {
public:
    ServiceTask(TaskManager* tm, const TaskContext& context);
 
    ~ServiceTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();
private:
    TaskManager* task_manager_;
    TaskDispatch* task_dispatch_;
};

}//namespace
#endif

