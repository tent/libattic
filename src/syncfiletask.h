#ifndef SYNCFILETASK_H_
#define SYNCFILETASK_H_
#pragma once

#include "tenttask.h"

class SyncFileTask : public TentTask
{
    int SyncMetaData();
public:
    SyncFileTask( TentApp* pApp,
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

    ~SyncFileTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

private:


};

#endif

