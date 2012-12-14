

#ifndef SYNCMANIFESTTASK_H_
#define SYNCMANIFESTTASK_H_
#pragma once

#include "tenttask.h"

class SyncManifestTask : public TentTask
{
public:
    SyncManifestTask( TentApp* pApp, 
                      FileManager* pFm, 
                      ConnectionManager* pCon, 
                      const AccessToken& at,
                      const std::string& entity,
                      const std::string& filepath,
                      const std::string& tempdir, 
                      const std::string& workingdir,
                      const std::string& configdir,
                      void (*callback)(int, void*));

    ~SyncManifestTask();
    
    void RunTask();
 
};


#endif

