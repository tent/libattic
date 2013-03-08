#include "threadworker.h"

#include <iostream>
#include <boost/thread/thread.hpp>

#include "task.h"
#include "taskqueue.h"

ThreadWorker::ThreadWorker()
{
    m_State = ThreadWorker::IDLE;
}

ThreadWorker::~ThreadWorker()
{
}

void ThreadWorker::Run()
{
    std::cout<<" thread worker starting ... " << std::endl;
    Task* pTask = NULL;
    while(!(GetState() == ThreadWorker::EXIT)) {
        if(GetState() == ThreadWorker::IDLE) {
            // Get a job
            pTask = CentralTaskQueue::GetInstance()->SyncPopFront();
        }

        if(pTask)  {
            SetState(ThreadWorker::RUNNING);
            PollTask(pTask);

            if(pTask->GetTaskState() == Task::FINISHED) {
                pTask = NULL;
                SetState(ThreadWorker::FINISHED);
            }
        }

        if(GetState() == ThreadWorker::FINISHED) {
            // Do some finished step, then idle
            SetState(ThreadWorker::IDLE);
        }

        boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
    }
    std::cout<<" thread  worker ending ... " << std::endl;
}

void ThreadWorker::PollTask(Task* pTask)
{
    std::cout<<"polling task " << std::endl;
    switch(pTask->GetTaskState()) {
        case Task::IDLE:
            {
                std::cout<<" starting task " << std::endl;
                // Start the task
                pTask->OnStart();
                pTask->SetRunningState();
                break;
            }
        case Task::RUNNING:
            {
                std::cout<<" running task " << std::endl;
                pTask->RunTask();
                break;
            }
        case Task::PAUSED:
            {
                std::cout<< " task paused " << std::endl;
                pTask->OnPaused();
                break;
            }
        case Task::FINISHED:
            {
                std::cout<< " task finished " << std::endl;
                pTask->OnFinished();
                break;
            }
        default:
            {
                std::cout<<" default " << std::endl;
                break;
            }
    };
}

int ThreadWorker::GetState()
{
    int t;
    this->Lock();
    t = m_State;
    this->Unlock();
    return t;
}

void ThreadWorker::SetState(ThreadState t)
{
    Lock();
    m_State = t;
    Unlock();
}

void ThreadWorker::SetThreadExit()
{
    this->Lock();
    m_State = ThreadWorker::EXIT;
    this->Unlock();
}
