#ifndef SYNCFILETASK_H_
#define SYNCFILETASK_H_
#pragma once

#include <string>
#include <map>

#include "tenttask.h"
#include "filepost.h"
#include "fileinfo.h"

class TaskDelegate;
class SyncFileTask : public TentTask {
    int SyncMetaData(FilePost& out);
    /*
    int AssessFileInfo(const FileInfo& local_fi, const FileInfo& meta_fi);
    int Pull(const AtticPost& p, const FileInfo& meta_fi);
    */
    int ProcessFileInfo(const FilePost& p);
    int RetrieveChunkInfo(const FilePost& post, FileInfo* fi);
public:
    SyncFileTask(FileManager* pFm,
                 CredentialsManager* pCm,
                 const AccessToken& at,
                 const Entity& entity,
                 const std::string& filepath,
                 const std::string& tempdir,
                 const std::string& workingdir,
                 const std::string& configdir,
                 TaskDelegate* callbackDelegate);

    ~SyncFileTask();

    virtual void OnStart(); 
    virtual void OnPaused(); 
    virtual void OnFinished();

    void RunTask();

    void Cb(int a, void* b);

private:
    std::map<std::string, bool> m_ProcessingQueue;
    std::string m_PostID;
};

#endif

