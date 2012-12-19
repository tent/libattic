
#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include <deque>
#include <pthread.h>

#include "mutexclass.h"
#include "threading.h"
#include "taskqueue.h"

class Task;
class TaskQueue;

class TaskArbiter
{
public:
    TaskArbiter();
    ~TaskArbiter();

    void SpinOffTask(Task* pTask); // Spin off detached thread

private:
    TaskQueue              m_TaskQueue;
    std::deque<pthread_t>  m_ThreadHandles; 

};


#endif

