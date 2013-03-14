#ifndef POLLTASK_H_
#define POLLTASK_H_
#pragma once

#include <map>
#include <string>

#include "tenttask.h"
#include "folder.h"
#include "taskdelegate.h"

class PollDelegate;

class PollTask : public TentTask {
    int SyncFolderPosts();
    int SyncFolder(Folder& folder);
    int GetFolderPostCount();

    void PushBackFile(const std::string& filepath);
    void RemoveFile(const std::string& filepath);
    bool IsFileInQueue(const std::string& filepath);

public:
    void PollTaskCB(int a, std::string& b);

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
              const TaskDelegate* callbackDelegate);
 
    ~PollTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

private:
    std::map<std::string, bool> m_ProcessingQueue; // Files currently being processed

    PollDelegate* m_pDelegate;
};

class PollDelegate : public TaskDelegate {
public:
    PollDelegate(PollTask* p){
        m_pTask = p;
    }
    ~PollDelegate(){}

    virtual void Callback(const int type,
                          const int code,
                          const int state,
                          const std::string& var) const
    {
        if(m_pTask) {
            std::string retval = var;
            m_pTask->PollTaskCB(code, retval);
        }

        if(this)
            delete this;
    }

private:
    PollTask* m_pTask;
};

#endif

