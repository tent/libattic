#include "threading.h"

#include <iostream>
#include <deque>
#include <unistd.h>

#include "errorcodes.h"
#include "task.h"
#include "taskqueue.h"


void* ThreadFunc(void* arg)
{
    if(arg)                                                                          
    {                                                                                
        std::cout<<" setting up thread " << std::endl;
        // Setup thread                                                              
        ThreadData* pTd = (ThreadData*)arg;                                          
        if(pTd)
        {
            pTd->Lock();
            TaskQueue* pTq = pTd->GetTaskQueue();
            pthread_t handle = pTd->GetThreadHandle();
            pTd->Unlock();

            // Local task queue
            typedef std::deque<Task*> TaskQueue;
            TaskQueue LocalTaskQueue;

            //Task* pTask = NULL;                                                  
            std::cout<<"Thread : " << handle <<" running til done..."<< std::endl;

            for(;;)                                                                  
            {                                                                        
                if(!pTd)
                {
                    std::cout<<" INVALID THREAD DATA " << std::endl;
                }

                pTd->Lock();
                int State = pTd->GetThreadState()->GetThreadState();                             
                pTd->Unlock();                                                  

                if(State == ThreadState::EXIT || State == ThreadState::FINISHED)
                {
                    std::cout<<"Thread : " << handle << " Got exit signal! " << std::endl;
                    break;
                }

                unsigned int size = LocalTaskQueue.size();
                if(size)
                {
                    //std::cout<<"thread : " << handle << " aquired task " << std::endl;
                    if(State != ThreadState::RUNNING)                                
                    {                                                                
                        while(pTd->TryLock()) { sleep(0); }                    
                        pTd->GetThreadState()->SetStateRunning();                                
                        pTd->Unlock();                                          
                    }

                    // If we have things in the queue, lets run them, 
                    // if they are in the running state.
                    Task* pTask = NULL;
                    TaskQueue::iterator itr = LocalTaskQueue.begin();
                    for(;itr != LocalTaskQueue.end(); itr++)
                    {
                        pTask = (*itr);
                        if(pTask)
                        {
                            std::cout<<" switching " << std::endl;
                            switch(pTask->GetTaskState())
                            {
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
                                        // Run the task
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
                                        // Remove from LocalTaskQueue
                                        (*itr) = NULL;
                                        itr = LocalTaskQueue.erase(itr);
                                        itr--;
                                        std::cout<< " new size : " << LocalTaskQueue.size() << std::endl;
                                        break;

                                    }
                                default:
                                    {
                                        std::cout<<" default " << std::endl;
                                    }
                                    break;

                            };
                        }
                        else
                        {
                            std::cout<<" INVALID " << std::endl;
                        }
                        sleep(0); // sleep between iterations
                    }
                }
                else
                {
                    // Grab some tasks
                    Task* pTask = pTq->SyncPopFront();                                

                    if(!pTask && (State != ThreadState::IDLE))                       
                    {                                                                
                        std::cout <<"Thread : " << handle << " Didn't aquire a task, idling ... " << std::endl;
                        pTd->Lock();                    
                        pTd->GetThreadState()->SetStateIdle();                                   
                        pTd->Unlock();                                          
                    } 
                    else
                    {
                        if(pTask)
                            LocalTaskQueue.push_back(pTask);
                    }

                }
            }                                                                        

            pTd->Lock();    
            pTd->GetThreadState()->SetStateFinished();
            pTd->Unlock();                             
        }
    }    

    std::cout << " thread exiting ... " << std::endl;
    g_ThreadCount--;                                                                 
    pthread_exit(NULL);                                                              
}

ThreadPool::ThreadPool()
{                                               
    m_TaskQueue = NULL;
}                                               

ThreadPool::~ThreadPool()
{
    if(m_TaskQueue)
        m_TaskQueue = NULL;
}

void ThreadPool::SetTaskQueue(TaskQueue* pQueue)
{
    if(pQueue)
    {
        m_TaskQueue = pQueue;
    }
    else
    {
        std::cout<<" Invalid Task Queue ... " << std::endl;
    }

}

int ThreadPool::Initialize()
{

    return ret::A_OK;
}

int ThreadPool::Shutdown()
{
    std::cout<<" sending exit signals " << std::endl;
    std::cout<<" thread count : " << m_ThreadData.size() << std::endl;
    for(unsigned int i=0; i<m_ThreadData.size(); i++)
    {
        while(m_ThreadData[i]->TryLock()) { sleep(0); }
        m_ThreadData[i]->GetThreadState()->SetStateExit();
        m_ThreadData[i]->Unlock();
    }

    std::cout<<" joining threads " << std::endl;
    for(unsigned int i=0; i<m_ThreadHandles.size(); i++)
    {
        pthread_join(m_ThreadHandles[i], NULL);
    }

    std::cout<<" cleaning up thread data " << std::endl;
    for(unsigned int i=0; i<m_ThreadData.size(); i++)
    {
        if(m_ThreadData[i])
        {
            std::cout<<"right here"<< std::endl;
            delete m_ThreadData[i];
            m_ThreadData[i] = NULL;
        }
    }

    std::cout<<" SIZE : " << m_ThreadData.size();
    m_ThreadData.clear();

    return ret::A_OK;
}

int ThreadPool::ExtendPool(unsigned int stride)
{
    int status = ret::A_OK;

    for(unsigned int i=0; i < stride; i++)
    {
        ThreadData* pData = new ThreadData();
        while(m_TaskQueue->TryLock()) { sleep(0); }
        pData->SetTaskQueue(m_TaskQueue);
        m_TaskQueue->Unlock();

        pthread_t Handle;                                                                          
        int rc = pthread_create(&Handle, NULL, ThreadFunc, (void*)pData);

        if(rc)
        {
            // Error spinning off task
            std::cout<< " error spinning off task " << std::endl;
            status = ret::A_FAIL_CREATE_THREAD;
        }
        else
        {
            m_ThreadHandles.push_back(Handle);
            
            while(pData->TryLock()) { sleep(0); }
            pData->SetThreadHandle(Handle);
            m_ThreadData.push_back(pData);
            m_ThreadCount++;
            pData->Unlock();
        }
    }

    return status;
}

int ThreadPool::AbridgePool(unsigned int stride)
{
    int status = ret::A_OK;
    /*
    for(unsigned int i=0; i < stride; i++)
    {
        while(m_ThreadData[i]->State.TryLock()) { sleep(0); }
        m_ThreadData[i]->State.SetStateExit();
        m_ThreadData[i]->State.Unlock();
        m_ThreadCount--;
    }
    */

    return status;
}

