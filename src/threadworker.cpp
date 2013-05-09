#include "threadworker.h"

#include <iostream>
#include <boost/thread/thread.hpp>


#include "sleep.h"
#include "taskarbiter.h"

namespace attic {

ThreadWorker::ThreadWorker(bool strict) {
    state_ = ThreadWorker::IDLE;
    strict_ = strict;
}

ThreadWorker::~ThreadWorker() {}

// Once a thread is set to exist state, it cannot/shouldnot be changed back
void ThreadWorker::Run() {
    std::cout<<" thread worker starting ... " << std::endl;
    Task* task = NULL;
    while(!(state() == ThreadWorker::EXIT)) {
        if(state() == ThreadWorker::IDLE) {
            // Get a job
            task = RetrieveTask();
            if(!task) { sleep::sleep_seconds(1); }
        }

        if(task)  {
            if(state() != ThreadWorker::SHUTDOWN)
                SetState(ThreadWorker::RUNNING);
            else 
                task->SetFinishedState();
            PollTask(&task);
        }

        if(state() == ThreadWorker::FINISHED) {
            std::cout<<" THREAD WORKER FINISHED " << std::endl;
            // Do some finished step, then idle
            SetState(ThreadWorker::IDLE);
        }

        if(state() == ThreadWorker::SHUTDOWN) {
            std::cout<<" THREAD EXIT " << std::endl;
            SetThreadExit();
            if(task) {
                std::cout<<" TASK STILL EXISTS?!?!?!?! " << std::endl;
                task->OnFinished();
                TaskArbiter::GetInstance()->ReclaimTask(task);
                task = NULL;
            }
        }
        sleep::sleep_milliseconds(100);
    }

    std::cout<<" thread  worker ending ... " << std::endl;
    if(state() == ThreadWorker::EXIT) {
        if(task) { 
//            delete task;
 //           task = NULL;
            task->OnFinished();
            TaskArbiter::GetInstance()->ReclaimTask(task);
            task = NULL;
        }
    }
    
}

void ThreadWorker::PollTask(Task** task) {
    switch((*task)->state()) {
        case Task::IDLE:
            {
                std::cout<<" starting task " << std::endl;
                // Start the task
                (*task)->OnStart();
                (*task)->SetRunningState();
                break;
            }
        case Task::RUNNING:
            {
                //std::cout<<" running task " << std::endl;
                (*task)->RunTask();
                sleep::sleep_milliseconds(100);
                break;
            }
        case Task::PAUSED:
            {
                std::cout<< " task paused " << std::endl;
                (*task)->OnPaused();
                sleep::sleep_milliseconds(100);
                break;
            }
        case Task::FINISHED:
            {
                std::cout<< " task finished " << std::endl;
                (*task)->OnFinished();
                TaskArbiter::GetInstance()->ReclaimTask(*task);
                (*task) = NULL;
                // cleanup task
                SetState(ThreadWorker::FINISHED);
                break;
            }
        default:
            {
                std::cout<<" default " << std::endl;
                break;
            }
    };
}

int ThreadWorker::state() {
    int t;
    Lock();
    t = state_;
    Unlock();
    return t;
}

void ThreadWorker::SetState(ThreadState t) {
    Lock();
    if(state_ != ThreadWorker::EXIT)
        state_ = t;
    Unlock();
}

void ThreadWorker::SetThreadExit() {
    Lock();
    state_ = ThreadWorker::EXIT;
    Unlock();
}

void ThreadWorker::SetThreadShutdown() {
    Lock();
    state_ = ThreadWorker::SHUTDOWN;
    Unlock();
}

void ThreadWorker::SetTaskPreference(Task::TaskType type, bool active) {
    task_preference_[type] = active;
}

Task* ThreadWorker::RetrieveTask() {
    Task* t = NULL;
    // Retrieve task based on preference first, then just any old task
    PreferenceMap::iterator itr = task_preference_.begin();
    for(;itr!= task_preference_.end(); itr++) {
        if(itr->second)
            t = TaskArbiter::GetInstance()->RequestTask(itr->first);
    }

    if(!t && !strict_)
        t = TaskArbiter::GetInstance()->RequestTask();
    return t;
}

ThreadWorkerFactory::ThreadWorkerFactory() {
    push_pull.first = 0;
    push_pull.second = true; // strict 
    pull_push.first = 0;
    pull_push.second = true; // strict
    poll.first = 0;
    poll.second = true; // strict
    rename_delete.first = 0;
    rename_delete.second = false; // not strict
    sync.first = 0;
    sync.second = false; // not strict
    generic.first = 0;
    generic.second = false; // not strict

    thread_count_ = 0;
}

ThreadWorkerFactory::~ThreadWorkerFactory() {}

ThreadWorker* ThreadWorkerFactory::GetThreadWorker() {

}



}//namespace
