
#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include <deque>

#include <pthread.h>

class Task;

class TaskArbiter
{
public:
    TaskArbiter();
    ~TaskArbiter();


    void SpinOffTask(Task* pTask); // Spin off detached thread


private:
    std::deque<pthread_t>  m_ThreadHandles; 
    volatile unsigned int m_ThreadCount; // current thread count

};



#endif

