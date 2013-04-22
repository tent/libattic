#ifndef RENAMETASK_H_
#define RENAMETASK_H_
#pragma once

#include <string>
#include "tenttask.h"

namespace attic { 

class RenameTask : public TentTask {
    int RenameFile(const std::string& old_filepath, const std::string& new_filepath);
    int RenameFolder(const std::string& folderpath);
public:
    RenameTask(FileManager* pFm, 
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context,
               TaskDelegate* callbackDelegate);

    ~RenameTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {} 
    virtual void OnFinished() {}

    void RunTask();
private:
};

}//namespace
#endif

