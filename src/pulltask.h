#ifndef PULLTASK_H_
#define PULLTASK_H_
#pragma once

#include <string>

#include "tenttask.h"
#include "response.h"
#include "post.h"

class TaskDelegate;

class PullTask: public TentTask {
    int PullFile(const std::string& filepath);
public:
    PullTask( TentApp* pApp, 
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

    ~PullTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {} 
    virtual void OnFinished() {}

    void RunTask();

private:
};

#endif

