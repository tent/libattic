
#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>

#include "tenttask.h"

class PushTask : public TentTask
{
    int PushFile(const std::string& filepath);
    int PostFile(const std::string& url, const std::string &filepath, FileInfo* fi);
    int PutFile(const std::string& url, const std::string &filepath, FileInfo* fi);
public:
    PushTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*));

    ~PushTask();

    void RunTask();

private:
};

#endif

