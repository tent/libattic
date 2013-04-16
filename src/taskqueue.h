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
    TaskQueue(){task_queue_.clear();}

    ~TaskQueue(){}                                                                 

    void SyncPushBack(Task* pTask) { 
        if(!pTask) {
            std::cout<<" INVALID TASK PASSED " << std::endl;
            return;
        }

        Lock();
        if(pTask) {
             std::cout<< "^^^\t pushing back task ... of type : "<< pTask->type() << std::endl;
             task_queue_.push_back(pTask);
             std::cout<<" queue size : " << task_queue_.size() << std::endl;
        }
        Unlock();
    }

    Task* SyncPopFront() {                                                                              
        Task* pTask = NULL;                                                        
        
        Lock();
        if(task_queue_.size() > 0) {
            std::cout<<" Popping off task ... "<< task_queue_.size() << std::endl;
            pTask = task_queue_.front();                                           
            task_queue_[0] = NULL;
            task_queue_.pop_front();
            std::cout<<" size now : " << task_queue_.size() << std::endl;            
            
        }                                                                          
        Unlock();

        return pTask;                                                              
    }                                                                              

private:                                                                           
    std::deque<Task*> task_queue_;                                                 
};


class CentralTaskQueue : public TaskQueue {
    CentralTaskQueue() {}
    CentralTaskQueue(const CentralTaskQueue& rhs){}
    CentralTaskQueue operator=(const CentralTaskQueue& rhs){ return *this;}
public:
    ~CentralTaskQueue(){}

    void Shutdown() {
        if(instance_) {
            delete instance_;
            instance_ = NULL;
        }
    }

    static CentralTaskQueue* GetInstance() {
        if(!instance_)
            instance_ = new CentralTaskQueue();
        return instance_;
    }


private:
    static CentralTaskQueue* instance_;
};

}//namespace
#endif

