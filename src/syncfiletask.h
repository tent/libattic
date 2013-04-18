#ifndef SYNCFILETASK_H_
#define SYNCFILETASK_H_
#pragma once

#include <string>
#include <map>

#include "tenttask.h"
#include "filepost.h"
#include "fileinfo.h"

namespace attic { 

class TaskDelegate;
class SyncFileTask : public TentTask {
    int SyncMetaData(FilePost& out);
    
    int ProcessFileInfo(const FilePost& p);
    int RaisePullRequest(const FilePost& p, FileInfo& fi);
public:
    SyncFileTask(FileManager* pFm,
                 CredentialsManager* pCm,
                 const AccessToken& at,
                 const Entity& entity,
                 const TaskContext& context,
                 TaskDelegate* callbackDelegate);

    ~SyncFileTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

    void Cb(int a, void* b);

private:
    std::map<std::string, bool> processing_queue_;
    std::string post_id_;
};

}//namespace
#endif

