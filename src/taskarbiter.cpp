#include "taskarbiter.h"

#include "task.h"
#include "taskqueue.h"
#include "pulltask.h"
#include "threading.h"

TaskArbiter::TaskArbiter()
{
    m_pTaskQueue = NULL;
    m_pTaskQueue = new TaskQueue();

    m_pPool = new ThreadPool();
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

int TaskArbiter::Initialize(unsigned int poolSize)
{
    if(m_pPool)
    {
        m_pPool->Initialize();
        m_pPool->SetTaskQueue(m_pTaskQueue);
        m_pPool->ExtendPool(poolSize);
    }
}

int TaskArbiter::Shutdown()
{
    if(m_pPool)
    {
        m_pPool->Shutdown();
    }

    return ret::A_OK;
}

// Spin off detached thread, (not explicitly detached,
// but treated as such, in all actuallity on most platforms
// its probably joinable)
void TaskArbiter::SpinOffTask(Task* pTask)
{
    m_pTaskQueue->PushBack(pTask);
}


