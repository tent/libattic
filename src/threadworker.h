#ifndef THREADWORKER_H_
#define THREADwORKER_H_
#pragma once

#include "mutexclass.h"

class Task;

class ThreadWorker : public MutexClass
{
    enum ThreadState
    {
        IDLE = 0,
        RUNNING,
        EXIT,
        FINISHED
    };

    void PollTask(Task* pTask);
    void SetState(ThreadState t);
public:

    ThreadWorker();
    ~ThreadWorker();

    void SetThreadExit();
    int GetState();

    void Run();
private:

    ThreadState m_State;
};
#endif

