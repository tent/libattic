#ifndef POLLTASK_H_
#define POLLTASK_H_
#pragma once

#include "tenttask.h"

class PollTask : public TentTask
{

    int SyncFolderPosts();
    int GetAtticPostCount();
public:
    PollTask( TentApp* pApp,
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
              void (*callback)(int, void*));

    ~PollTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

private:

};

#endif
