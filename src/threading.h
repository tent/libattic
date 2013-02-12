
#ifndef THREADING_H_
#define THREADING_H_
#pragma once

#include <vector>
#include <pthread.h>

#include "mutexclass.h"

class TaskQueue;

class ThreadState                                                         
{                                                                                                
public:                                                                                          
    enum RunningState                                                                            
    {                                                                                            
        IDLE = 0,                                                                                
        RUNNING,                                                                                 
        EXIT,
        FINISHED
    };

public:                                                                                          
    ThreadState(){ m_State = IDLE; }                                               
    ~ThreadState(){}                                                                             

    void SetStateIdle()     { if(m_State != EXIT) m_State = IDLE; } 
    void SetStateRunning()  { if(m_State != EXIT) m_State = RUNNING; }
    void SetStateExit()     { m_State = EXIT; }
    void SetStateFinished() { m_State = FINISHED; }

    int GetThreadState() { return m_State; }                                               

private:                                                                                         
    RunningState m_State;                                                                        
};                                                                                               

class ThreadData  : public MutexClass
{                    
public:
    ThreadData() {}
    ~ThreadData() {}

    ThreadState* GetThreadState()   { return &m_State; }
    TaskQueue* GetTaskQueue()       { return m_pTq; }
    pthread_t GetThreadHandle()     { return m_Handle; }   

    void SetTaskQueue(TaskQueue* pTq) { m_pTq = pTq; }
    void SetThreadHandle(pthread_t handle) { m_Handle = handle; }

private:
    ThreadState m_State;                                                
    TaskQueue*  m_pTq;                                                                             
    pthread_t   m_Handle;                                                                          
};                                                                                               

class ThreadPool                                                                                 
{                                                                                                
public:                                                                                          
    ThreadPool();
    ~ThreadPool();

    int Initialize();
    int Shutdown();

    void SetTaskQueue(TaskQueue* pQueue);
    int ExtendPool(unsigned int stride);                                                        
    int AbridgePool(unsigned int stride);                                                       

private:                                                                                         
    std::vector<pthread_t>    m_ThreadHandles;
    std::vector<ThreadData*>  m_ThreadData;                                                       
    TaskQueue* m_TaskQueue;
    unsigned int m_ThreadCount;                                                                  
};                                                                                               


// TODO :: remove this, just for testing
volatile static unsigned int g_ThreadCount = 0;

#endif

