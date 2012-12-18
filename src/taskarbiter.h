
#ifndef TASKARBITER_H_
#define TASKARBITER_H_
#pragma once

#include <deque>

#include <pthread.h>


#include <iostream> // temp remove



class MutexClass
{
public:
    MutexClass()
    {
       pthread_mutex_init(&m_Mutex, NULL);
    }

    virtual ~MutexClass()
    {
        pthread_mutex_destroy(&m_Mutex);
    }

    int TryLock() 
    {
        std::cout << " locking ... " << std::endl; return pthread_mutex_trylock(&m_Mutex); 
    } // as all things unix 0 is ok
    int Unlock() 
    {
        std::cout << " unlocking ... " << std::endl;  return pthread_mutex_unlock(&m_Mutex);
    }

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

volatile static unsigned int g_ThreadCount = 0;

#endif

