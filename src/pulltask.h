

#ifndef PULLTASK_H_
#define PULLTASK_H_
#pragma once

#include <string>

#include "tentapp.h"
#include "task.h"

class TentApp;
class FileManager;
class ConnectionManager;

class PullTask: public Task
{
    int PullFile(const std::string& filepath);
    int GetFileAndWriteOut(const std::string& url, const std::string &filepath);

public:
    PullTask();
    PullTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir);

    ~PullTask();

    virtual void RunTask();

private:
   AccessToken          m_At;

   std::string          m_Entity;
   std::string          m_Filepath;
   std::string          m_TempDirectory;

   TentApp*             m_pTentApp; 
   FileManager*         m_pFileManager;
   ConnectionManager*   m_pConnectionManager;
};

#endif

