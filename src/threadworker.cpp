#include "threadworker.h"

#include <iostream>
#include <boost/thread/thread.hpp>

#include "task.h"
#include "taskqueue.h"
#include "sleep.h"

namespace attic {

ThreadWorker::ThreadWorker() {
    state_ = ThreadWorker::IDLE;
}

ThreadWorker::~ThreadWorker() {}

// Once a thread is set to exist state, it cannot/shouldnot be changed back
void ThreadWorker::Run() {
    std::cout<<" thread worker starting ... " << std::endl;
    Task* task = NULL;
    while(!(state() == ThreadWorker::EXIT)) {
        if(state() == ThreadWorker::IDLE) {
            // Get a job
            task = CentralTaskQueue::GetInstance()->SyncPopFront();
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
                delete task;
                task = NULL;
            }
        }

        //std::cout<<" ping state : " << state() << std::endl;

        boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
    }
    std::cout<<" thread  worker ending ... " << std::endl;
    if(state() == ThreadWorker::EXIT) {
        if(task) { 
            task->OnFinished();
            delete task;
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
                // cleanup task
                if((*task)) { 
                    delete (*task);
                    (*task) = NULL;
                }
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

}//namespace
