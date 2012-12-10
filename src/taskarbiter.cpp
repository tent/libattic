#include "taskarbiter.h"

#include <iostream>

TaskArbiter::TaskArbiter()
{
    m_ThreadCount = 0;

}

TaskArbiter::~TaskArbiter()
{

}


void* ThreadFunc(void* arg)
{
    if(arg)
    {
        Task* pTask = (Task*)arg;
        pTask->RunTask();
    }
    else
        std::cout<<"invalid arg passed to thread"<<std::endl;

    pthread_exit(NULL);
}

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
void TaskArbiter::SpinOffTask(Task* pTask)
{
    if(!pTask)
        return;

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, ThreadFunc, (void*)pTask);

    if(rc)
    {
        std::cout<<"ERROR SPINNING OFF A TASK : " << rc << std::endl;
        return;
    }

    m_ThreadHandles.push_back(thread);
}
