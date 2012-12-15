

#ifndef PULLTASK_H_
#define PULLTASK_H_
#pragma once

#include <string>

#include "tenttask.h"

class PullTask: public TentTask
{
    int PullFile(const std::string& filepath);
    int GetFileAndWriteOut(const std::string& url, const std::string &filepath);

public:
    PullTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              CredentialsManager* pCm,
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir,
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*));

    ~PullTask();

    void RunTask();

private:
   
};

#endif

