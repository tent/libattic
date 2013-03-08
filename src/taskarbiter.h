#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include "mutexclass.h"

class Task;
class ThreadPool;

class TaskArbiter : public MutexClass
{

    TaskArbiter();
    TaskArbiter(const TaskArbiter& rhs) {}
    TaskArbiter operator=(const TaskArbiter& rhs) { return *this; }
public:
    ~TaskArbiter();

    int Initialize(unsigned int poolSize);
    int Shutdown();

    static TaskArbiter* GetInstance();

    int SpinOffTask(Task* pTask); // Spin off detached thread

private:
    static bool            m_bInitialized;
    static TaskArbiter*    m_pInstance;
    ThreadPool*            m_pPool;
};


#endif

