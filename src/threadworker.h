#ifndef THREADWORKER_H_
#define THREADwORKER_H_
#pragma once

#include <map>
#include <utility>
#include "mutexclass.h"
#include "task.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "accesstoken.h"
#include "entity.h"

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
    ThreadWorker(FileManager* fm,
                 CredentialsManager* cm,
                 const AccessToken& at,
                 const Entity ent,
                 bool strict = false);

    ~ThreadWorker();
    void SetTaskPreference(Task::TaskType type, bool active = true);

    void SetThreadExit();
    void SetThreadShutdown();
    int state();

    void Run();
private:
    TaskFactory task_factory_;
    typedef std::map<Task::TaskType, bool> PreferenceMap;
    PreferenceMap task_preference_;
    ThreadState state_;
    bool strict_; // If the worker is strict, it will only take its prefered tasks
                  // otherwise it will check its preference first, before just taking
                  // anything it can get

    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;
    AccessToken             access_token_;
    Entity                  entity_;
};

}//namespace
#endif

