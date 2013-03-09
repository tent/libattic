#ifndef POLLTASK_H_
#define POLLTASK_H_
#pragma once

#include "tenttask.h"
#include "folder.h"

#include <map>
#include <string>

class PollTask : public TentTask {
    int SyncFolderPosts();
    int SyncFolder(Folder& folder);
    int GetFolderPostCount();

    void PushBackFile(const std::string& filepath);
    void RemoveFile(const std::string& filepath);
    bool IsFileInQueue(const std::string& filepath);

public:
    void PollTaskCB(int a, void* b);

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
    bool m_bRunning;

    std::map<std::string, bool> m_ProcessingQueue; // Files currently being processed
};

#endif
