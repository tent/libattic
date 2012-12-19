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

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
void TaskArbiter::SpinOffTask(Task* pTask)
{
    /*
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
    */

    ThreadData* pData = new ThreadData();
    pData->pTq = &m_TaskQueue;

    while(!m_TaskQueue.TryLock()) { sleep(0); }
    m_TaskQueue.PushBack(pTask);
    m_TaskQueue.Unlock();
}


