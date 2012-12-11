#include "taskarbiter.h"

#include <iostream>

#include "task.h"
#include "pulltask.h"



TaskArbiter::TaskArbiter()
{
}

TaskArbiter::~TaskArbiter()
{

}

void* ThreadFunc(void* arg)
{
    std::cout<<"thread func"<<::std::endl;
    if(arg)
    {
        std::cout<<"attempting to run task"<<std::endl;
        Task* pTask = (Task*)arg;

        pTask->RunTask();
    }
    else
        std::cout<<"invalid arg passed to thread"<<std::endl;

    std::cout<<"thread exiting ... " << std::endl;
    g_ThreadCount--;
    std::cout << " G THREAD COUNT : " << g_ThreadCount << std::endl;
    pthread_exit(NULL);
}

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
void TaskArbiter::SpinOffTask(Task* pTask)
{
    std::cout<<"Spinning off task..."<<std::endl;
    if(!pTask)
        return;

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, ThreadFunc, (void*)pTask);

    if(rc)
    {
        std::cout<<"ERROR SPINNING OFF A TASK : " << rc << std::endl;
        return;
    }

    std::cout<< " Created thread : " << thread << std::endl;

    g_ThreadCount++;
    std::cout<<" G THREAD COUNT : " << g_ThreadCount << std::endl;
    m_ThreadHandles.push_back(thread);
}
