#ifndef UPLOADTASK_H_
#define UPLOADTASK_H_
#pragma once

#include "tenttask.h"

namespace attic {

class UploadTask : public TentTask { 
    int ProcessFile(const std::string& post_id);
public:
    UploadTask(FileManager* fm,
               CredentialsManager* cm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);
    ~UploadTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

}// namespace

#endif

