#ifndef PUSHPUBLICTASK_H_
#define PUSHPUBLICTASK_H_
#pragma once

#include "tenttask.h"
#include "downloadpost.h"

namespace attic {

class Connection;

class PushPublicTask : public TentTask {
    int PushFile(const std::string& filepath, DownloadPost& out);

    bool GenerateDownloadPost(const std::string& filepath, DownloadPost& out);
    void WriteFileToConnection(const std::string& filepath, Connection* con);
    void WriteOnceFileToConnection(const std::string& filepath, Connection * con);

    void GeneratePublicLink(DownloadPost& in, std::string& link_out);

    void CallbackWithUrl(int code, std::string url, std::string error) {
        if(context_.delegate()) {
            if(context_.delegate()->type() == TaskDelegate::REQUEST) {
                RequestDelegate* p = static_cast<RequestDelegate*>(context_.delegate());
                p->Callback(code, url.c_str(), error.c_str());
            }
        }
    }


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

