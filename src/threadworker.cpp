#include "threadworker.h"

#include <iostream>
#include <boost/thread/thread.hpp>


#include "sleep.h"
#include "taskarbiter.h"

namespace attic {
ThreadWorker::ThreadWorker(FileManager* fm,
                           CredentialsManager* cm,
                           const AccessToken& at,
                           const Entity& ent,
                           bool strict) {
    state_ = ThreadWorker::IDLE;
    strict_ = strict;

    file_manager_ = fm;
    credentials_manager_ = cm;
    access_token_ = at;
    entity_ = ent;

    task_factory_.Initialize(file_manager_, credentials_manager_, access_token_, entity_);

    thread_ = NULL;
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
            // Do some finished step, then idle
            SetState(ThreadWorker::IDLE);
        }

        if(state() == ThreadWorker::SHUTDOWN) {
            SetThreadExit();
            RelinquishTask(&task);
        }
        sleep::sleep_milliseconds(100);
    }

    std::cout<<" thread  worker ending ... " << std::endl;
    if(state() == ThreadWorker::EXIT) {
        RelinquishTask(&task);
    }
    std::cout<<" worker exiting ... " << std::endl;
}

void ThreadWorker::PollTask(Task** task) {
    switch((*task)->state()) {
        case Task::IDLE:
            {
                // Start the task
                (*task)->OnStart();
                (*task)->SetRunningState();
                break;
            }
        case Task::RUNNING:
            {
                (*task)->RunTask();
                sleep::sleep_milliseconds(100);
                break;
            }
        case Task::PAUSED:
            {
                (*task)->OnPaused();
                sleep::sleep_milliseconds(100);
                break;
            }
        case Task::FINISHED:
            {
                (*task)->OnFinished();
                task_factory_.ReclaimTask(*task);
                (*task) = NULL;
                // cleanup task
                SetState(ThreadWorker::FINISHED);
                break;
            }
        default:
            {
                break;
            }
    };
}

int ThreadWorker::state() {
    int t;
    state_mtx_.Lock();
    t = state_;
    state_mtx_.Unlock();
    return t;
}

void ThreadWorker::SetState(ThreadState t) {
    state_mtx_.Lock();
    if(state_ != ThreadWorker::EXIT)
        state_ = t;
    state_mtx_.Unlock();
}

void ThreadWorker::SetThreadExit() {
    state_mtx_.Lock();
    state_ = ThreadWorker::EXIT;
    state_mtx_.Unlock();
}

void ThreadWorker::SetThreadShutdown() {
    state_mtx_.Lock();
    state_ = ThreadWorker::SHUTDOWN;
    state_mtx_.Unlock();
}

void ThreadWorker::SetTaskPreference(Task::TaskType type, bool active) {
    task_preference_[type] = active;
}

Task* ThreadWorker::RetrieveTask() {
    bool success = false;
    TaskContext tc;
    // Retrieve task based on preference first, then just any old task
    PreferenceMap::iterator itr = task_preference_.begin();
    for(;itr!= task_preference_.end(); itr++) {
        if(itr->second) { 
            success = TaskArbiter::GetInstance()->RequestTaskContext(itr->first, tc);
            if(success) {
                break;
            }
        }
    }

    if(!success && !strict_) {
        if(strict_) { std::cout << " STRICT THREAD WORKER REQUESTING RANDOM TASK ! " << std::endl; } 
        success = TaskArbiter::GetInstance()->RequestTaskContext(tc);
    }

    Task* t = NULL;
    if(success) { 
        t = task_factory_.GetTentTask(tc);
    }

    return t;
}

void ThreadWorker::RelinquishTask(Task** task) {
    if(*task) { 
        std::cout<<" shutting down task ... : "<< (*task)->type() << std::endl;
        (*task)->OnFinished();
        task_factory_.ReclaimTask(*task);
        (*task) = NULL;
    }
}



}//namespace
