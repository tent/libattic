#ifndef DELETETASK_H_
#define DELETETASK_H_
#pragma once

#include "tenttask.h"

class FileInfo;
class TaskDelegate;

class DeleteTask : public TentTask {
public:
    DeleteTask( TentApp* pApp, 
                FileManager* pFm, 
                CredentialsManager* pCm,
                TaskArbiter* pTa,
                TaskFactory* pTf,
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

#endif

