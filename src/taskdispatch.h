#ifndef TASKDISPATCH_H_
#define TASKDISPATCH_H_
#pragma once

#include <string>
#include "task.h"
#include "tenttask.h"
#include "taskcontext.h"
#include "taskmanager.h"
#include "filemanager.h"
#include "taskfactory.h"
#include "credentialsmanager.h"

namespace attic { 

class TaskDispatch {
    int CreateAndSpinOffTask(const TaskContext& tc);
public:
    TaskDispatch(FileManager* fm,
                 CredentialsManager* cm,
                 const AccessToken& at,
                 const Entity& entity,
                 const std::string& tempdir, 
                 const std::string& workingdir,
                 const std::string& configdir);

    ~TaskDispatch();

    void Initialize();
    void Shutdown();

    void Process(TaskManager* tm);
    void Dispatch();
    
    void set_dispatch_queue(const TaskContext::ContextQueue& q) { dispatch_queue_ = q; } 
private:
    TaskContext::ContextQueue dispatch_queue_;
    TaskContext::ContextQueue hold_queue_;

    // General settings for dispatch
    TaskFactory             task_factory_;
    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;

    AccessToken             access_token_;
    Entity                  entity_;

    std::string             temp_directory_;
    std::string             working_directory_;
    std::string             config_directory_;
};

} //namespace
#endif

