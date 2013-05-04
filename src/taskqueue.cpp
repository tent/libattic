#include "taskqueue.h"

#include <iostream>

namespace attic {
TaskQueue::TaskQueue() {}
TaskQueue::~TaskQueue(){
    std::cout<< " task queue destructor " << std::endl;
}

void TaskQueue::ClearTaskQueue() { 
    std::cout<<" before lock " << std::endl;
    Lock();
    std::cout<<" after lock " << std::endl;
    if(task_queue_.size()) {
        for(unsigned int i=0; i<task_queue_.size(); i++) {
            Task* p = task_queue_[i];
            task_queue_[i] = NULL;
            if(p) {
                delete p;
                p = NULL;
            }
        }
    }
    Unlock();
}

void TaskQueue::SyncPushBack(Task* pTask) { 
    if(!pTask) {
        std::cout<<" INVALID TASK PASSED " << std::endl;
        return;
    }

    Lock();
    if(pTask) {
         std::cout<< "^^^\t pushing back task ... of type : "<< pTask->type() << std::endl;
         task_queue_.push_back(pTask);
         queue_stats_.increment_total_tasks();
    }
    Unlock();
}

Task* TaskQueue::SyncPopFront() {
    Task* pTask = NULL;                                                        
    
    Lock();
    if(task_queue_.size() > 0) {
        std::cout<<" Popping off task ... "<< task_queue_.size() << std::endl;
        pTask = task_queue_.front();                                           
        task_queue_[0] = NULL;
        task_queue_.pop_front();
        std::cout<<" size now : " << task_queue_.size() << std::endl;            
        queue_stats_.increment_active_tasks();
    }                                                                          
    Unlock();
    return pTask;                                                              
}                                                                              

void TaskQueue::ReclaimTask(Task* task) {
    std::cout<<" RECLAMING TASK : " << task << std::endl;
    if(task) {
        delete task;
        task = NULL;
        queue_stats_.increment_reclaimed_tasks();
        queue_stats_.decrement_active_tasks();
    }
}

void TaskQueue::PrintStats() {
    queue_stats_.PrintStats();
}

unsigned int TaskQueue::ActiveTaskCount() {
    return queue_stats_.active_tasks();
}

QueueStats::QueueStats() {
    total_tasks_ = 0;
    active_tasks_ = 0;
    reclaimed_tasks_ = 0;
}

QueueStats::~QueueStats() {}

void QueueStats::PrintStats() {
    std::cout<<" Task Queue Stats ------------" <<std::endl;
    std::cout<<"\ttotal tasks : " << total_tasks() << std::endl;
    std::cout<<"\tactive tasks : " << active_tasks() << std::endl;
    std::cout<<"\treclaimed tasks : " << reclaimed_tasks() << std::endl;
}

// TODO :: come back at some point and rethink this, may be too much interleiving, look into 
//         atomic memory barriers, maybe c++11 atomic or boost atomic
unsigned int QueueStats::total_tasks() {
    t_mtx_.Lock();
    unsigned int retval = total_tasks_;
    t_mtx_.Unlock();
    return retval;
}

unsigned int QueueStats::active_tasks() {
    a_mtx_.Lock();
    unsigned int retval = active_tasks_;
    a_mtx_.Unlock();
    return retval;
}

unsigned int QueueStats::reclaimed_tasks() {
    r_mtx_.Lock();
    unsigned int retval = reclaimed_tasks_;
    r_mtx_.Unlock();
    return retval;
}

void QueueStats::increment_total_tasks() {
    t_mtx_.Lock();
    total_tasks_++;
    t_mtx_.Unlock();
}

void QueueStats::increment_active_tasks() {
    a_mtx_.Lock();
    active_tasks_++;
    a_mtx_.Unlock();
}

void QueueStats::decrement_active_tasks() {
    a_mtx_.Lock();
    active_tasks_--;
    a_mtx_.Unlock();
}

void QueueStats::increment_reclaimed_tasks() {
    r_mtx_.Lock();
    reclaimed_tasks_++;
    r_mtx_.Unlock();
}


}//namespace

