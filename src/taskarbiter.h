
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
public:
    TaskArbiter();
    ~TaskArbiter();

    int Initialize(unsigned int poolSize);
    int Shutdown();

    void SpinOffTask(Task* pTask); // Spin off detached thread

private:
    TaskQueue*             m_pTaskQueue;
    ThreadPool*            m_pPool;

};


#endif

