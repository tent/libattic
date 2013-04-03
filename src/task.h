#ifndef TASK_H_
#define TASK_H_
#pragma once

#include "errorcodes.h"

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
    Task(TaskType type = UNKNOWN) {
        m_State = IDLE;
        m_Type = type;
    }                                                  
public:                                                       
    virtual ~Task(){}
    virtual void RunTask() = 0;                               

    virtual void OnStart() = 0;
    virtual void OnPaused() = 0;
    virtual void OnFinished() = 0;

    TaskState GetTaskState() const { return m_State; }
    TaskType GetTaskType() const { return m_Type; }

    virtual void SetIdleState() { m_State = IDLE; }
    virtual void SetRunningState() { m_State = RUNNING; }
    virtual void SetPausedState() { m_State = PAUSED; }
    virtual void SetFinishedState() { m_State = FINISHED; }

private:
    TaskType    m_Type;
    TaskState   m_State;
};                                                            

}//namespace 
#endif

