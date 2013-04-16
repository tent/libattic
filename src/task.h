#ifndef TASK_H_
#define TASK_H_
#pragma once

#include "errorcodes.h"
#include "taskcontext.h"

namespace attic {

// Inherit from this to implement specific tasks 
class Task {                                                             
public:
    enum TaskState {
        IDLE=0,
        RUNNING,
        PAUSED,
        FINISHED
    };

    enum TaskType {                                                                   
        // TentTask
        UNKNOWN=0,
        PUSH,
        PULL,
        PULLALL,
        DELETE,                                                         
        SYNC,
        SYNC_FILE_TASK,
        ENCRYPT,
        DECRYPT,
        // ManifestTask
        QUERYMANIFEST,
        SCANDIRECTORY,
        // Polling
        POLL,
        // Service
        SERVICE
    };
protected:
    Task(const TaskContext& context, TaskType type = UNKNOWN) {
        context_ = context;
        state_ = IDLE;
        type_ = type;
    }                                                  
public:                                                       
    virtual ~Task(){}
    virtual void RunTask() = 0;                               

    virtual void OnStart() = 0;
    virtual void OnPaused() = 0;
    virtual void OnFinished() = 0;

    const TaskState& state() const { return state_; }
    const TaskType& type() const { return type_; }

    virtual void SetIdleState() { state_ = IDLE; }
    virtual void SetRunningState() { state_ = RUNNING; }
    virtual void SetPausedState() { state_ = PAUSED; }
    virtual void SetFinishedState() { state_ = FINISHED; }

    void SetContext(const TaskContext& context) { context_ = context; }
    void PushBackContextValue(const std::string& key, const std::string& value) {
        context_.set_value(key, value);
    }
protected:
    TaskContext context_;

private:
    TaskType    type_;
    TaskState   state_;
};                                                            

}//namespace 
#endif

