#ifndef UPLOADTASK_H_
#define UPLOADTASK_H_
#pragma once

#include "tenttask.h"

namespace attic {

class UploadTask : public TentTask { 
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

