#ifndef RENAMETASK_H_
#define RENAMETASK_H_
#pragma once

#include "tenttask.h"

namespace attic { 

class RenameTask : public TentTask {
public:
    RenameTask(FileManager* pFm, 
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context,
               TaskDelegate* callbackDelegate);

    ~RenameTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {} 
    virtual void OnFinished() {}

    void RunTask();
private:
};

}//namespace
#endif

