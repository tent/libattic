

#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_
#pragma once

#include <deque>
#include "mutexclass.h"
#include "task.h"

class TaskQueue : public MutexClass                                                
{                                                                                  
public:                                                                            
    TaskQueue(){}                                                                  
    ~TaskQueue(){}                                                                 

    void PushBack(Task* pTask) { if(pTask) m_TaskQueue.push_back(pTask); }     

    Task* PopFront()                                                           
    {                                                                              
        Task* pTask = NULL;                                                        
        if(!m_TaskQueue.empty())                                                   
        {                                                                          
            pTask = m_TaskQueue.front();                                           
            m_TaskQueue.pop_front();                                               
        }                                                                          
        return pTask;                                                              
    }                                                                              

private:                                                                           
    std::deque<Task*> m_TaskQueue;                                                 
};

#endif

