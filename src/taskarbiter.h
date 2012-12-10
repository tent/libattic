
#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include <deque>

#include <pthread.h>


class MutexClass
{
public:
    MutexClass(){}
    ~MutexClass(){}

    int TryLock() { return pthread_mutex_trylock(&m_Mutex); } // as all things unix 0 is ok
    int Unlock() { return pthread_mutex_unlock(&m_Mutex); }

private:
    pthread_mutex_t m_Mutex;

};

class Task;

class TaskArbiter
{
public:
    TaskArbiter();
    ~TaskArbiter();


    void SpinOffTask(Task* pTask); // Spin off detached thread

private:
    std::deque<pthread_t>  m_ThreadHandles; 

};



#endif

