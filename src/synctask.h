

#ifndef SYNCTASK_H_
#define SYNCTASK_H_
#pragma once

#include "tenttask.h"

class SyncTask : public TentTask
{
    int GetAtticPostCount();
    int SyncAtticPosts();
public:
    SyncTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*));

    ~SyncTask();

    void RunTask();


};

#endif

