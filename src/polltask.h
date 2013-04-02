#ifndef POLLTASK_H_
#define POLLTASK_H_
#pragma once

#include <map>
#include <string>

#include "tenttask.h"
#include "folder.h"
#include "taskdelegate.h"
#include "event.h"
#include <boost/timer/timer.hpp>
namespace attic { 

class PollDelegate;

class PollTask : public TentTask, public event::EventListener {
    int SyncFolderPosts();
    int SyncFolder(Folder& folder);
    int GetFolderPostCount();

    void PushBackFile(const std::string& filepath);
    void RemoveFile(const std::string& filepath);
    bool IsFileInQueue(const std::string& filepath);

public:
    void PollTaskCB(int a, std::string& b);

    PollTask(FileManager* pFm,
             CredentialsManager* pCm,
             const AccessToken& at,
             const Entity& entity,
             const std::string& filepath,
             const std::string& tempdir,
             const std::string& workingdir,
             const std::string& configdir,
             TaskDelegate* callbackDelegate);
 
    ~PollTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    virtual void OnEventRaised(const event::Event& event);
    void RunTask();

private:
    std::map<std::string, bool> m_ProcessingQueue; // Files currently being processed

    PollDelegate* m_pDelegate;
    boost::timer::cpu_timer::cpu_timer timer_;

    bool running_;
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
    }


private:
    PollTask* m_pTask;
};

}//namespace
#endif

