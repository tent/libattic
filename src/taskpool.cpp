#include "taskpool.h"

namespace attic {

TaskPool::TaskPool() 
{
}

TaskPool::~TaskPool()
{
    TaskMap::iterator itr = m_TaskMap.begin();

    for(;itr != m_TaskMap.end(); itr++)
    {
        for(;itr->second.size(); )
        {
            Task* pTask = itr->second.front();
            delete pTask;
            pTask = NULL;
            itr->second.pop_front();
        }
    }
}

void TaskPool::PushBack(Task* pTask)
{
    Lock();
    if(pTask)
        m_TaskMap[pTask->GetTaskType()].push_back(pTask);
    Unlock();
}

Task* TaskPool::Remove(Task* pTask)
{
    if(pTask)
    {
        Lock();
        // Find
        TaskQueue::iterator itr = FindTask(pTask, pTask->GetTaskType());
        if(itr != m_TaskMap[pTask->GetTaskType()].end())
        {
            // Remove
            pTask = *itr;                
            *itr = NULL;
        }
        Unlock();
    }

    return pTask;
}

TaskPool::TaskQueue::iterator TaskPool::FindTask(Task* pTask, Task::TaskType type)
{
    TaskQueue::iterator itr = m_TaskMap[type].begin();
    if(pTask)
    {
        for(;itr != m_TaskMap[type].end(); itr++)
        {
            if((*itr) == pTask)
                break;
        }
    }
    else
        itr = m_TaskMap[type].end();

    return itr;
}

}//namespace
