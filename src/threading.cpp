#include "threading.h"

#include <iostream>
#include <unistd.h>

#include "errorcodes.h"
#include "task.h"
#include "taskqueue.h"


void* ThreadFunc(void* arg)
{
    std::cout<<" Starting thread ... " << std::endl;
    if(arg)                                                                          
    {                                                                                
        std::cout<<" setting up thread " << std::endl;
        // Setup thread                                                              
        ThreadData* pTd = (ThreadData*)arg;                                          

        if(pTd)
        {                                                                            
            std::cout<<" attempting to aquire task queue " << std::endl;
            while(pTd->TryLock()) { sleep(0); }
            TaskQueue* pTq = pTd->GetTaskQueue();
            pthread_t handle = pTd->GetThreadHandle();
            pTd->Unlock();

            Task* pTask = NULL;                                                  

            std::cout<<"Thread : " << handle <<" running til done..."<< std::endl;
            for(;;)                                                                  
            {                                                                        
                if(!pTd)
                {
                    std::cout<<" INVALID THREAD DATA " << std::endl;
                }

                while(pTd->TryLock()) { sleep(0); }                            
                int State = pTd->GetThreadState()->GetThreadState();                             
                pTd->Unlock();                                                  

                if(State == ThreadState::EXIT || State == ThreadState::FINISHED)
                {
                    std::cout<<"Thread : " << handle << " Got exit signal! " << std::endl;
                    break;
                }

                if(pTask)                                                            
                {                                                                    
                    std::cout<<"thread : " << handle << " aquired task " << std::endl;
                    if(State != ThreadState::RUNNING)                                
                    {                                                                
                        while(pTd->TryLock()) { sleep(0); }                    
                        pTd->GetThreadState()->SetStateRunning();                                
                        pTd->Unlock();                                          
                    }                                                                

                    std::cout<<"Thread : " << handle << " About to run a task " << std::endl;

                    pTask->SetRunningState();                                        
                    pTask->RunTask();                                                
                    pTask->SetFinishedState();                                       
                    pTask = NULL;                                                    
                }                                                                    
                else                                                                 
                {                                                                    
                    // Grab a task -- TODO :: maybe grab several? ;)                 
                    //std::cout<<"Thread : " << handle << " Attempting to aquire task ... " << std::endl;
                    //while(pTq->TryLock()) { sleep(0); }                        
                    pTask = pTq->PopFront();                                
                    //pTq->Unlock();                                              

                    if(!pTask && (State != ThreadState::IDLE))                       
                    {                                                                
                        std::cout <<"Thread : " << handle << " Didn't aquire a task, idling ... " << std::endl;
                        while(pTd->TryLock()) { sleep(0); }                    
                        pTd->GetThreadState()->SetStateIdle();                                   
                        pTd->Unlock();                                          
                    } 
                }                                                                    
                sleep(0);                                                            
            }                                                                        

            while(pTd->TryLock()) { sleep(0); }    
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

