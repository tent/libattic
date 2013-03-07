#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

TaskArbiter* TaskArbiter::m_pInstance = 0;
bool TaskArbiter::m_bInitialized = false;

TaskArbiter::TaskArbiter()
{
    //m_pTaskQueue = NULL;
    //m_pTaskQueue = new TaskQueue();
    m_pPool = new ThreadPool();
}

TaskArbiter::~TaskArbiter()
{
    /*
    if(m_pTaskQueue)
    {
        delete m_pTaskQueue;
        m_pTaskQueue = NULL;
    }
    */

    if(m_pPool) {
        delete m_pPool;
        m_pPool = NULL;
    }
}

int TaskArbiter::Initialize(unsigned int poolSize)
{
    int status = ret::A_OK;
    std::cout<<" init lock ... " << std::endl;
    Lock();
    if(m_pPool)  {
        m_pPool->Initialize();
        //m_pPool->SetTaskQueue(m_pTaskQueue);
        m_pPool->SetTaskQueue(CentralTaskQueue::GetInstance());
        m_pPool->ExtendPool(poolSize);
        m_bInitialized = true;
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }
    Unlock();
    std::cout<<" init unlock ... " << std::endl;
    return status; 
}

int TaskArbiter::Shutdown()
{
    int status = ret::A_OK;


    std::cout<<"s lock ... " << std::endl;
    Lock();
    if(m_pPool)
        status = m_pPool->Shutdown();
    Unlock();
    std::cout<<" s unlock ... " << std::endl;

    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }

    CentralTaskQueue::GetInstance()->Shutdown();

    return status;
}

TaskArbiter* TaskArbiter::GetInstance()
{
    if(!m_pInstance)
        m_pInstance = new TaskArbiter();
    return m_pInstance;
}

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
int TaskArbiter::SpinOffTask(Task* pTask)
{
    int status = ret::A_OK;

    if(m_bInitialized) { 
        std::cout<<"... pusing back task ... " << std::endl;
        CentralTaskQueue::GetInstance()->SyncPushBack(pTask);
    }
    else
        status = ret::A_FAIL_SUBSYSTEM_NOT_INITIALIZED;

    return status;
}


