#ifndef METATASK_H_
#define METATASK_H_
#pragma once

#include <string>
#include "tenttask.h"

namespace attic {

class MetaTask : public TentTask {
    int RetrieveFileInfoHistory(const std::string& post_id);
public:
    MetaTask(FileManager* fm, 
             CredentialsManager* cm,
             const AccessToken& at,
             const Entity& entity,
             const TaskContext& context);

    ~MetaTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();

};

} // Namespace

#endif

