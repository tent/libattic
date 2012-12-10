#include "taskarbiter.h"

#include <iostream>

#include "task.h"

volatile static unsigned int g_ThreadCount = 0;

TaskArbiter::TaskArbiter()
{
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

    g_ThreadCount--;
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

    g_ThreadCount++;
    m_ThreadHandles.push_back(thread);
}
