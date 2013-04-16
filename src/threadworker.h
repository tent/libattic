#ifndef THREADWORKER_H_
#define THREADwORKER_H_
#pragma once

#include "mutexclass.h"

namespace attic {

class Task;

class ThreadWorker : public MutexClass {
    enum ThreadState {
        IDLE = 0,
        RUNNING,
        EXIT,
        FINISHED
    };

    void PollTask(Task* pTask);
    void SetState(ThreadState t);
public:
    ThreadWorker();
    ~ThreadWorker();

    void SetThreadExit();
    int state();

    void Run();
private:
    ThreadState state_;
};

}//namespace
#endif

