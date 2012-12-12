

#ifndef SYNCTASK_H_
#define SYNCTASK_H_
#pragma once

#include "tenttask.h"

class SyncPostsTask : public TentTask
{
    int GetAtticPostCount();
    int SyncAtticPosts();
public:
    SyncPostsTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*));

    ~SyncPostsTask();

    void RunTask();


};

#endif

