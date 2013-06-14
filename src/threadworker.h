#ifndef THREADWORKER_H_
#define THREADwORKER_H_
#pragma once

#include <map>
#include <utility>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"
#include "task.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "accesstoken.h"
#include "entity.h"

namespace attic {

class ThreadWorker {
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
    void RelinquishTask(Task** task);
    
    void Run();
public:
    ThreadWorker(FileManager* fm,
                 CredentialsManager* cm,
                 const AccessToken& at,
                 const Entity& ent,
                 bool strict = false);

    ~ThreadWorker();
    void SetTaskPreference(Task::TaskType type, bool active = true);

    void SetThreadExit();
    void SetThreadShutdown();
    int state();


    void StartThread() { 
        if(!thread_) {
            std::cout<<" starting worker thread ... " << std::endl;
            thread_ = new boost::thread(&ThreadWorker::Run, this);
        }
    }

    void StopThread() {
        if(thread_) {
            std::cout<<" exiting worker thread .. " << std::endl;
            SetThreadExit();
            thread_->join();
            delete thread_;
            thread_ = NULL;
        }
    }

    boost::thread* thread() { return thread_; }
private:
    TaskFactory task_factory_;
    typedef std::map<Task::TaskType, bool> PreferenceMap;

    PreferenceMap task_preference_;
    MutexClass state_mtx_;
    ThreadState state_;
    bool strict_; // If the worker is strict, it will only take its prefered tasks
                  // otherwise it will check its preference first, before just taking
                  // anything it can get
    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;
    AccessToken             access_token_;
    Entity                  entity_;
    boost::thread* thread_;
};

}//namespace
#endif

