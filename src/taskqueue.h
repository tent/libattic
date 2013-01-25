

#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_
#pragma once

#include <deque>
#include "mutexclass.h"
#include "task.h"

#include <iostream>
class TaskQueue : public MutexClass                                                
{                                                                                  
public:                                                                            
    TaskQueue(){m_TaskQueue.clear();}                                                                  

    ~TaskQueue(){}                                                                 

    void SyncPushBack(Task* pTask) 
    { 
        while(TryLock()) { sleep(0); }
        if(pTask) m_TaskQueue.push_back(pTask);
        Unlock();
    }

    Task* SyncPopFront()                                                           
    {                                                                              
        while(TryLock()) { sleep(0); }
        Task* pTask = NULL;                                                        
        
        if(m_TaskQueue.size() > 0)                                                   
        {                                                                          
            std::cout<<" Popping off task ... "<< m_TaskQueue.size() << std::endl;
            pTask = m_TaskQueue.front();                                           
            m_TaskQueue[0] = NULL;
            m_TaskQueue.pop_front();
            std::cout<<" size now : " << m_TaskQueue.size() << std::endl;            
            
        }                                                                          
        Unlock();
        return pTask;                                                              
    }                                                                              

private:                                                                           
    std::deque<Task*> m_TaskQueue;                                                 
};

#endif

