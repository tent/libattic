

#ifndef PULLTASK_H_
#define PULLTASK_H_
#pragma once

#include <string>

#include "tenttask.h"

class PullTask: public TentTask
{
    int GetChunkPost(FileInfo* fi, Response& responseOut);

    int PullFileNew(const std::string& filepath);
    int RetreiveFile( const std::string filepath, 
                      const std::string postpath, 
                      const Credentials& fileCred,
                      Post& post,
                      FileInfo* fi);

    int RetreiveFileCredentials(FileInfo* fi, Credentials& out);
    int RetrieveAttachment( const std::string& url, 
                            const AccessToken* at, 
                            std::string& outBuffer);

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
              void (*callback)(int, void*));

    ~PullTask();

    virtual void OnStart() { } 
    virtual void OnPaused() { } 
    virtual void OnFinished() { }

    void RunTask();

    int GetDownloadSpeed();

private:
   
};

#endif

