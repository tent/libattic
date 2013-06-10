#ifndef TASKPOOL_H_
#define TASKPOOL_H_
#pragma once

#include <deque>
#include <map>

#include "task.h"
#include "mutexclass.h"
#include "taskcontext.h"

namespace attic {

class TaskPool : public MutexClass {
    typedef std::map<int, TaskContext::ContextQueue> ContextMap;
public:
    TaskPool();
    ~TaskPool();

    void ClearPool();

    void PushBack(const TaskContext& tc);

    bool HasTaskContextOfType(const Task::TaskType type);
    bool RequestTaskContext(const Task::TaskType type, TaskContext& out);
    bool RequestNextAvailableTaskContext(TaskContext& out);

    unsigned int ActiveTaskCount();
        
    TaskContext::ContextQueue* operator[](const Task::TaskType type) {
        return &context_map_[type];
    }

    std::string stats() {
        std::ostringstream stats;
        ContextMap::iterator itr = context_map_.begin();
        for(;itr!= context_map_.end(); itr++) {
            stats << " task type : " << itr->first << " task count : " << itr->second.size() << std::endl;
        }
        return stats.str();
    }
private:
    unsigned int total_task_count_;
    unsigned int active_task_count_;
    unsigned int reclaimed_task_count_;
    ContextMap context_map_;
};

}//namespace
#endif

