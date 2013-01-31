#ifndef TASK_H_
#define TASK_H_
#pragma once

#include <iostream>
#include "errorcodes.h"

class Task // Inherit from this to implement specific tasks   
{                                                             
public:
    enum TaskState
    {
        IDLE=0,
        RUNNING,
        PAUSED,
        FINISHED
    };

protected:
    Task()
    {
        m_State = IDLE;
    }                                                  
public:                                                       
    virtual ~Task(){}
    virtual void RunTask() = 0;                               

    virtual void OnStart() = 0;
    virtual void OnPaused() = 0;
    virtual void OnFinished() = 0;

    int GetTaskState() { return m_State; }
    void GetTaskState(TaskState state) { m_State = state; }

    virtual void SetIdleState() { m_State = IDLE; }
    virtual void SetRunningState() { m_State = RUNNING; }
    virtual void SetPausedState() { m_State = PAUSED; }
    virtual void SetFinishedState() { m_State = FINISHED; }

private:
    TaskState   m_State;
};                                                            


#endif

