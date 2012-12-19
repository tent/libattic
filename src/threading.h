
#ifndef THREADING_H_
#define THREADING_H_
#pragma once

#include <deque>
#include <pthread.h>

#include "mutexclass.h"

class TaskQueue;

class ThreadState : public MutexClass                                                            
{                                                                                                
public:                                                                                          
    enum RunningState                                                                            
    {                                                                                            
        IDLE = 0,                                                                                
        RUNNING,                                                                                 
        EXIT                                                                                     
    };                                                                                           
public:                                                                                          
    ThreadState(){ m_State = IDLE; }                                               
    ~ThreadState(){}                                                                             

    void SetStateIdle() { if(m_State != EXIT) m_State = IDLE; }                                  
    void SetStateRunning() { if(m_State != EXIT) m_State = RUNNING; }                            
    void SetStateExit() { m_State = EXIT; }                                                      

    int GetThreadState() { return m_State; }                                                     

private:                                                                                         
    RunningState m_State;                                                                        
};                                                                                               

struct ThreadData                                                                                
{                                                                                                
    ThreadState state;                                                                           
    TaskQueue*  pTq;                                                                             
    pthread_t   handle;                                                                          
};                                                                                               

class ThreadPool                                                                                 
{                                                                                                
public:                                                                                          
    ThreadPool(unsigned int nCount = 1);
    ~ThreadPool();

    void ExtendPool(unsigned int stride);                                                        
    void AbridgePool(unsigned int stride);                                                       

private:                                                                                         
    std::deque<ThreadData*>  m_ThreadData;                                                       
    unsigned int m_ThreadCount;                                                                  
};                                                                                               


// TODO :: remove this, just for testing
volatile static unsigned int g_ThreadCount = 0;

#endif

