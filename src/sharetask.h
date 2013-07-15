#ifndef SHARETASK_H_
#define SHARETASK_H_
#pragma once

#include "tenttask.h"

namespace attic {

class ShareTask : public TentTask {
    bool RerievePublicKey(const std::string& entity, std::string& out);
public:
    ShareTask(FileManager* fm, 
              CredentialsManager* cm,
              const AccessToken& at,
              const Entity& entity,
              const TaskContext& context);
    ~ShareTask();

    virtual void OnStart() {}
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

} // namespace
#endif

