#ifndef RENAMETASK_H_
#define RENAMETASK_H_
#pragma once

#include <string>
#include "tenttask.h"

namespace attic { 

class RenameTask : public TentTask {
    int RenameFile(const std::string& file_type,
                   const std::string& old_filepath, 
                   const std::string& new_filename);
    int RenameFolder(const std::string& file_type,
                     const std::string& old_folderpath, 
                     const std::string& new_foldername);
public:
    RenameTask(FileManager* pFm, 
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);

    ~RenameTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {} 
    virtual void OnFinished() {}

    void RunTask();
private:
};

}//namespace
#endif

