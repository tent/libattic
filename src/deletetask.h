#ifndef DELETETASK_H_
#define DELETETASK_H_
#pragma once

#include "tenttask.h"

namespace attic { 

class FileInfo;
class TaskDelegate;

class DeleteTask : public TentTask {
public:
    DeleteTask(FileManager* pFm, 
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const std::string& filepath,
               const std::string& tempdir, 
               const std::string& workingdir,
               const std::string& configdir,
               TaskDelegate* callbackDelegate);

    ~DeleteTask();

    void RunTask();

    virtual void OnStart() { } 
    virtual void OnPaused() { } 
    virtual void OnFinished() { }
};

} //namespace
#endif

