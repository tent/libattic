
#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>

#include "tentapp.h"
#include "task.h"

class TentApp;
class FileInfo;
class FileManager;
class ConnectionManager;

class PushTask : public Task
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
              const std::string& tempdir);

    ~PushTask();

    void RunTask();

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

