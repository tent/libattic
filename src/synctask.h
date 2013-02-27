#ifndef SYNCTASK_H_
#define SYNCTASK_H_
#pragma once 

#include "tenttask.h"

class SyncTask : public TentTask
{
    int SyncMetaData();
    int GetAtticPostCount();
    int GetFilePosts();
    int GetChunkPosts();

    int InsertFileInfoToManager(const std::vector<FileInfo>& filist);
public:
    SyncTask( TentApp* pApp,
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

    ~SyncTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

private:

};

#endif

