
#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include <pthread.h>

#include "mutexclass.h"

class Task;
class TaskQueue;
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

    void SpinOffTask(Task* pTask); // Spin off detached thread

private:
    static TaskArbiter*    m_pInstance;
    TaskQueue*             m_pTaskQueue;
    ThreadPool*            m_pPool;
};


#endif

