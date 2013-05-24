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
            if(task) {
                task->OnFinished();
                task_factory_.ReclaimTask(task);
                task = NULL;
            }
        }
        sleep::sleep_milliseconds(100);
    }

    std::cout<<" thread  worker ending ... " << std::endl;
    if(state() == ThreadWorker::EXIT) {
        if(task) { 
            std::cout<<" shutting down task ... : " task.type() << std::endl;
            task->OnFinished();
            task_factory_.ReclaimTask(task);
            task = NULL;
        }
    }
    std::cout<<" worker exiting ... " << std::endl;
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
                task_factory_.ReclaimTask(*task);
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
    bool success = false;
    TaskContext tc;
    // Retrieve task based on preference first, then just any old task
    PreferenceMap::iterator itr = task_preference_.begin();
    for(;itr!= task_preference_.end(); itr++) {
        if(itr->second)
            success = TaskArbiter::GetInstance()->RequestTaskContext(itr->first, tc);
    }

    if(!success && !strict_)
        success = TaskArbiter::GetInstance()->RequestTaskContext(tc);

    Task* t = NULL;
    if(success)
        t = task_factory_.GetTentTask(tc);

    return t;
}



}//namespace
