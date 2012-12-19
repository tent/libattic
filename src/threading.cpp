#include "threading.h"

#include "task.h"
#include "taskqueue.h"

void* ThreadFunc(void* arg)
{
    if(arg)                                                                          
    {                                                                                
        // Setup thread                                                              
        ThreadData* pTd = (ThreadData*)arg;                                          
        if(pTd && pTd->pTq)                                                          
        {                                                                            
            for(;;)                                                                  
            {                                                                        
                while(!pTd->state.TryLock()) { sleep(0); }                            
                int state = pTd->state.GetThreadState();                             
                pTd->state.Unlock();                                                  

                if(state == ThreadState::EXIT)                                       
                    break;                                                           
                Task* pTask = NULL;                                                  

                if(pTask)                                                            
                {                                                                    
                    if(state != ThreadState::RUNNING)                                
                    {                                                                
                        while(!pTd->state.TryLock()) { sleep(0); }                    
                        pTd->state.SetStateRunning();                                
                        pTd->state.Unlock();                                          
                    }                                                                

                    pTask->SetRunningState();                                        
                    pTask->RunTask();                                                
                    pTask->SetFinishedState();                                       
                    pTask = NULL;                                                    
                }                                                                    
                else                                                                 
                {                                                                    
                    // Grab a task -- TODO :: maybe grab several? ;)                 
                    while(!pTd->pTq->TryLock()) { sleep(0); }                        
                    pTask = pTd->pTq->PopFront();                                
                    pTd->pTq->Unlock();                                              

                    if(!pTask && (state != ThreadState::IDLE))                       
                    {                                                                
                        while(!pTd->state.TryLock()) { sleep(0); }                    
                        pTd->state.SetStateIdle();                                   
                        pTd->state.Unlock();                                          
                    }                                                                
                }                                                                    

                sleep(0);                                                            
            }                                                                        
        }                                                                            
    }                                                                                

    g_ThreadCount--;                                                                 
    pthread_exit(NULL);                                                              
}

ThreadPool::ThreadPool(unsigned int nCount)
{                                               

}                                               

ThreadPool::~ThreadPool()
{

}

void ThreadPool::ExtendPool(unsigned int stride)
{

}

void ThreadPool::AbridgePool(unsigned int stride)
{

}

