#ifndef CONFIGTASK_H_
#define CONFIGTASK_H_
#pragma once

#include "tenttask.h"

/* Config task encapsulates generic configuration tasks
*/

namespace attic { 

class ConfigTask : public TentTask {
    bool AddRootDirectory(const std::string& directory_path);
    bool UnlinkRootDirectory(const std::string& directory_path);
    bool RemoveRootDirectory(const std::string& directory_path);
public:
    ConfigTask(FileManager* fm,
               CredentialsManager* cm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);

    ~ConfigTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();


    
};

} // namespace

#endif

