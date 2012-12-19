#include "threading.h"

#include <iostream>
#include <unistd.h>

#include "task.h"
#include "taskqueue.h"


void* ThreadFunc(void* arg)
{

    std::cout<<" Starting thread ... " << std::endl;
    if(arg)                                                                          
    {                                                                                
        // Setup thread                                                              
        ThreadData* pTd = (ThreadData*)arg;                                          

        if(pTd)
        {                                                                            
            while(!pTd->TryLock()) { sleep(0); }
            TaskQueue* pTq = pTd->GetTaskQueue();
            pTd->Unlock();


            for(;;)                                                                  
            {                                                                        
                while(!pTd->TryLock()) { sleep(0); }                            
                int State = pTd->GetThreadState()->GetThreadState();                             
                pTd->Unlock();                                                  

                if(State == ThreadState::EXIT || State == ThreadState::FINISHED)
                    break;

                Task* pTask = NULL;                                                  

                if(pTask)                                                            
                {                                                                    
                    if(State != ThreadState::RUNNING)                                
                    {                                                                
                        while(!pTd->TryLock()) { sleep(0); }                    
                        pTd->GetThreadState()->SetStateRunning();                                
                        pTd->Unlock();                                          
                    }                                                                

                    pTask->SetRunningState();                                        
                    pTask->RunTask();                                                
                    pTask->SetFinishedState();                                       
                    pTask = NULL;                                                    
                }                                                                    
                else                                                                 
                {                                                                    
                    // Grab a task -- TODO :: maybe grab several? ;)                 
                    while(!pTq->TryLock()) { sleep(0); }                        
                    pTask = pTq->PopFront();                                
                    pTq->Unlock();                                              

                    if(!pTask && (State != ThreadState::IDLE))                       
                    {                                                                
                        while(!pTd->TryLock()) { sleep(0); }                    
                        pTd->GetThreadState()->SetStateIdle();                                   
                        pTd->Unlock();                                          
                    }                                                                
                }                                                                    

                sleep(0);                                                            
            }                                                                        

            while(!pTd->TryLock()) { sleep(0); }    
            pTd->GetThreadState()->SetStateFinished();
            pTd->Unlock();                             
        }
    }    

    g_ThreadCount--;                                                                 
    pthread_exit(NULL);                                                              
}

ThreadPool::ThreadPool(TaskQueue* pQueue, unsigned int nCount)
{                                               
    if(pQueue)
    {
        m_TaskQueue = pQueue;
        ExtendPool(nCount);
    }
    else
    {
        std::cout<<" Invalid Task Queue ... " << std::endl;

    }

}                                               

ThreadPool::~ThreadPool()
{

    for(unsigned int i=0; i<m_ThreadData.size(); i++)
    {
        while(!m_ThreadData[i]->TryLock()) { sleep(0); }
        m_ThreadData[i]->GetThreadState()->SetStateExit();
        m_ThreadData[i]->Unlock();
    }

    
    //std::deque<ThreadData*>::iterator itr = 
    for(unsigned int i=0; i<m_ThreadData.size(); i++)
    {
        //while(!m_ThreadData[i]->State.TryLock()) { sleep(0); }
        if(m_ThreadData[i]->GetThreadState()->GetThreadState() == ThreadState::FINISHED)
        {
            std::cout<<"right here"<< std::endl;
            delete m_ThreadData[i];
            m_ThreadData[i] = NULL;
        }
        //m_ThreadData[i]->State.Unlock();
    }

    std::cout<<" SIZE : " << m_ThreadData.size();
    
    m_ThreadData.clear();



}

void ThreadPool::ExtendPool(unsigned int stride)
{
    for(unsigned int i=0; i < stride; i++)
    {
        ThreadData* pData = new ThreadData();
        pData->SetTaskQueue(m_TaskQueue);

        pthread_t Handle;                                                                          
        int rc = pthread_create(&Handle, NULL, ThreadFunc, (void*)pData);

        if(rc)
        {
            // Error spinning off task
        }
        else
        {
            while(!pData->TryLock()) { sleep(0); }
            pData->SetThreadHandle(Handle);
            m_ThreadData.push_back(pData);
            m_ThreadCount++;
            pData->Unlock();
        }
    }
}

void ThreadPool::AbridgePool(unsigned int stride)
{
    /*
    for(unsigned int i=0; i < stride; i++)
    {
        while(!m_ThreadData[i]->State.TryLock()) { sleep(0); }
        m_ThreadData[i]->State.SetStateExit();
        m_ThreadData[i]->State.Unlock();
        m_ThreadCount--;
    }
    */
}

