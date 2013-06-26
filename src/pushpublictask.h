#ifndef PUSHPUBLICTASK_H_
#define PUSHPUBLICTASK_H_
#pragma once

#include "tenttask.h"
#include "downloadpost.h"

namespace attic {

class Connection;

class PushPublicTask : public TentTask {
    int PushFile(const std::string& filepath);

    bool GenerateDownloadPost(const std::string& filepath, DownloadPost& out);
    void WriteFileToConnection(const std::string& filepath, Connection* con);
public:
    PushPublicTask(FileManager* fm,
                   CredentialsManager* cm,
                   const AccessToken& at,
                   const Entity& entity,
                   const TaskContext& context);
    ~PushPublicTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};


} // namespace

#endif

