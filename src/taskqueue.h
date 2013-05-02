#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_
#pragma once

#include <deque>
#include <iostream>

#include "mutexclass.h"
#include "task.h"

namespace attic {

class QueueStats {
public:
    QueueStats();
    ~QueueStats();

    void PrintStats();

    unsigned int total_tasks();
    unsigned int active_tasks();
    unsigned int reclaimed_tasks();

    void increment_total_tasks();
    void increment_active_tasks();
    void decrement_active_tasks();
    void increment_reclaimed_tasks();
private:
    MutexClass t_mtx_, a_mtx_, r_mtx_;
    unsigned int total_tasks_;
    unsigned int active_tasks_;
    unsigned int reclaimed_tasks_;
};

class TaskQueue : public MutexClass {
public:                                                                            
    TaskQueue();
    ~TaskQueue();

    void ClearTaskQueue();
    void SyncPushBack(Task* pTask);

    Task* SyncPopFront();
    void ReclaimTask(Task* task);

    void PrintStats();
    unsigned int ActiveTaskCount();
private:                                                                           
    std::deque<Task*> task_queue_;                                                 
    QueueStats queue_stats_;
};

}//namespace
#endif

