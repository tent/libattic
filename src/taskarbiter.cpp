#include "taskarbiter.h"

#include <iostream>

#include "task.h"
#include "pulltask.h"



TaskArbiter::TaskArbiter()
{
    std::cout<<"here"<<std::endl;
    m_pTaskQueue = NULL;
    m_pTaskQueue = new TaskQueue();

    m_pPool = new ThreadPool(m_pTaskQueue, 2);
}

TaskArbiter::~TaskArbiter()
{
    if(m_pTaskQueue)
    {
        delete m_pTaskQueue;
        m_pTaskQueue = NULL;
    }

    if(m_pPool)
    {
        delete m_pPool;
        m_pPool = NULL;
    }

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


    ThreadData* pData = new ThreadData();
    pData->pTq = m_pTaskQueue;

    */
    while(!m_pTaskQueue->TryLock()) { sleep(0); }
    m_pTaskQueue->PushBack(pTask);
    m_pTaskQueue->Unlock();

}


