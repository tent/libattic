#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>
#include <vector>

#include "tenttask.h"
/* 
 * Push task, begins the upload process of a file. The metadata post will be initialized or
 * updated here. Then an upload task will be spun off (event raised). The upload task does
 * actual uploading.
 *
 * The reason for this is to process as much metadata at once as possible, during the upload 
 * process metadata may change, it's better that these metadata changes are made as fast as 
 * possible as this determines where files are stored within the working directories across
 * devices. 
 *
 * This will propagate rename and move events faster and reduce the chance for folders and files
 * to re-materialize do to old metadata.
 */

namespace attic { 

class PushTask : public TentTask {
    
    int PushFile(const std::string& filepath);

public:
    PushTask(FileManager* pFm, 
             CredentialsManager* pCm,
             const AccessToken& at,
             const Entity& entity,
             const TaskContext& context);

    ~PushTask();

    virtual void OnStart() {} 
    virtual void OnPaused();
    virtual void OnFinished() {}

    int GetUploadSpeed();

    void RunTask();

private:
};

}//namespace
#endif

