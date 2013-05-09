#ifndef THREADWORKER_H_
#define THREADwORKER_H_
#pragma once

#include <map>
#include <utility>
#include "mutexclass.h"
#include "task.h"

namespace attic {

class ThreadWorker : public MutexClass {
    enum ThreadState {
        IDLE = 0,
        RUNNING,
        EXIT,
        FINISHED,
        SHUTDOWN,
    };

    void PollTask(Task** pTask);
    void SetState(ThreadState t);

    Task* RetrieveTask();
public:
    ThreadWorker(bool strict = false);
    ~ThreadWorker();
    void SetTaskPreference(Task::TaskType type, bool active = true);

    void SetThreadExit();
    void SetThreadShutdown();
    int state();

    void Run();
private:
    typedef std::map<Task::TaskType, bool> PreferenceMap;
    PreferenceMap task_preference_;
    ThreadState state_;
    bool strict_; // If the worker is strict, it will only take its prefered tasks
                  // otherwise it will check its preference first, before just taking
                  // anything it can get
};


// TODO :: tweak this later to churn out the correct ratio of workers
class ThreadWorkerFactory {
public:
    ThreadWorkerFactory();
    ~ThreadWorkerFactory();
    ThreadWorker* GetThreadWorker();
    void set_thread_count(unsigned int count) { thread_count_ = count; }
private:
    unsigned int thread_count_;
    typedef std::pair<unsigned int, bool> WorkerType;
    WorkerType push_pull;
    WorkerType pull_push;
    WorkerType poll;
    WorkerType rename_delete;
    WorkerType sync;
    WorkerType generic;
};

}//namespace
#endif

