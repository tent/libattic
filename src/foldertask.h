#ifndef FOLDERTASK_H_
#define FOLDERTASK_H_
#pragma once

#include "tenttask.h"

namespace attic {

class FolderTask : public TentTask{
    int CreateFolderPost(Folder& folder, std::string& id_out);
    int CreateFolder();

    int DeleteFolder();
public:
    FolderTask(FileManager* pFm,
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);

    ~FolderTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

}//namespace
#endif

