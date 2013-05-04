#ifndef POLLTASK_H_
#define POLLTASK_H_
#pragma once

#include <map>
#include <string>
#include <deque>

#include <boost/timer/timer.hpp>

#include "tenttask.h"
#include "folderpost.h"
#include "filepost.h"
#include "taskdelegate.h"
#include "event.h"
#include "censushandler.h"
#include "pagepost.h"

namespace attic { 

class PollDelegate;

class PollTask : public TentTask, public event::EventListener {
    void PushBackFile(const std::string& filepath);
    void RemoveFile(const std::string& filepath);
    bool IsFileInQueue(const std::string& filepath);

    int SyncFiles(std::deque<FilePost>& file_list);
public:
    void PollTaskCB(int a, std::string& b);

    PollTask(FileManager* pFm,
             CredentialsManager* pCm,
             const AccessToken& at,
             const Entity& entity,
             const TaskContext& context);
 
    ~PollTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    virtual void OnEventRaised(const event::Event& event);
    void RunTask();

private:
    std::map<std::string, bool> processing_queue_; // Files currently being processed
    PollDelegate* delegate_;
    boost::timer::cpu_timer timer_;

    bool running_;
    CensusHandler* census_handler_;
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
                          const std::string& var) const {
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

