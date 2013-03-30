#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_
#pragma once

#include <deque>
#include <iostream>

#include "mutexclass.h"
#include "task.h"

namespace attic {

class TaskQueue : public MutexClass {
public:                                                                            
    TaskQueue(){m_TaskQueue.clear();}

    ~TaskQueue(){}                                                                 

    void SyncPushBack(Task* pTask) 
    { 
        if(!pTask) {
            std::cout<<" INVALID TASK PASSED " << std::endl;
            return;
        }

        Lock();
        if(pTask) {
             std::cout<< "^^^\t pushing back task ... of type : "<< pTask->GetTaskType() << std::endl;
             m_TaskQueue.push_back(pTask);
             std::cout<<" queue size : " << m_TaskQueue.size() << std::endl;
        }
        Unlock();
    }

    Task* SyncPopFront()                                                           
    {                                                                              
        Task* pTask = NULL;                                                        
        
        Lock();
        if(m_TaskQueue.size() > 0) {
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


class CentralTaskQueue : public TaskQueue
{
    CentralTaskQueue() {}
    CentralTaskQueue(const CentralTaskQueue& rhs){}
    CentralTaskQueue operator=(const CentralTaskQueue& rhs){ return *this;}
public:
    ~CentralTaskQueue(){}

    void Shutdown() 
    {
        if(m_pInstance) {
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

    static CentralTaskQueue* GetInstance()
    {
        if(!m_pInstance)
            m_pInstance = new CentralTaskQueue();
        return m_pInstance;
    }


private:
    static CentralTaskQueue* m_pInstance;
};

}//namespace
#endif

