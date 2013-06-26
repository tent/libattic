#ifndef PUSHPUBLICTASK_H_
#define PUSHPUBLICTASK_H_
#pragma once

#include "tenttask.h"

namespace attic {

class PushPublicTask : public TentTask {
    int PushFile(const std::string& filepath);
public:
    PushPublicTask(FileManager* fm,
                   CredentialsManager* cm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context);
    ~PushPublicTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};


} // namespace

#endif

