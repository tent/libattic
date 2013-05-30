#ifndef UPLOADTASK_H_
#define UPLOADTASK_H_
#pragma once

#include "tenttask.h"
#include "fileinfo.h"

namespace attic {

class UploadTask : public TentTask { 
    int RetrieveFileInfo(const std::string& post_id, FileInfo& out);
    int ProcessFile(const FileInfo& fi);
public:
    UploadTask(FileManager* fm,
               CredentialsManager* cm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);
    ~UploadTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

}// namespace

#endif

