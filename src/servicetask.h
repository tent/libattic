#ifndef SERVICETASK_H_
#define SERVICETASK_H_
#pragma once

#include "tenttask.h"

namespace attic { 
// This task encapsulates general purpose systems that need dt updating
// Event system
// - Time sensative delete queues etc (future)
//
class ServiceTask : public TentTask {
public:
    ServiceTask(FileManager* pFm,
             CredentialsManager* pCm,
             const AccessToken& at,
             const Entity& entity,
             const std::string& filepath,
             const std::string& tempdir,
             const std::string& workingdir,
             const std::string& configdir,
             TaskDelegate* callbackDelegate);
 
    ~ServiceTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();
};

}//namespace
#endif

