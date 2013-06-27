#ifndef METATASK_H_
#define METATASK_H_
#pragma once

#include <string>
#include "tenttask.h"
#include "posttree.h"

namespace attic {

class MetaTask : public TentTask {
    void DownloadFileToLocation(const std::string& post_id, 
                                const std::string& version,
                                const std::string& folderpath);
    void RetrieveFileHistory(const std::string& filepath);
    int RetrieveFileInfoTree(const std::string& post_id, PostTree& out);
public:
    MetaTask(FileManager* fm, 
             CredentialsManager* cm,
             const AccessToken& at,
             const Entity& entity,
             const TaskContext& context);

    ~MetaTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

} // Namespace

#endif

