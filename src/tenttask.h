
#ifndef TENTTASK_H_
#define TENTTASK_H_
#pragma once

#include <string>

#include "tentapp.h"
#include "task.h"

#include "filemanager.h"
#include "connectionmanager.h"
#include "credentialsmanager.h"

class TentTask : public Task
{

public:
    TentTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              CredentialsManager* pCm,
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*))
    {
        m_pTentApp = pApp;
        m_pFileManager = pFm;
        m_pConnectionManager = pCon; 

        m_At = at;

        m_Entity = entity;
        m_Filepath = filepath;
        m_TempDirectory = tempdir;
        m_WorkingDirectory = workingdir;
        m_ConfigDirectory = configdir;

        mCallback = callback;
    }

    virtual ~TentTask()
    {

    }

    /*
    virtual void RunTask()
    {
    }
    */

    AccessToken* GetAccessToken() { return &m_At; }
    
    void GetEntity(std::string &out)            { out = m_Entity; }
    void GetFilepath(std::string &out)          { out = m_Filepath; }
    void GetTempDirectory(std::string &out)     { out = m_TempDirectory; } 
    void GetWorkingDirectory(std::string &out)  { out = m_WorkingDirectory; }
    void GetConfigDirectory(std::string &out)   { out = m_ConfigDirectory; }

    TentApp* GetTentApp()                       { return m_pTentApp; }
    FileManager* GetFileManager()               { return m_pFileManager; } 
    ConnectionManager* GetConnectionManager()   { return m_pConnectionManager; } 

    void SetCallback(void (*cb)(int, void*)) { mCallback = cb; }

protected:
    void Callback(int code, void* p)
    {
        std::cout<<" Callback " << std::endl;
        if(mCallback)
            mCallback(code, p); 

    }

private:
    AccessToken          m_At;

    std::string          m_Entity;
    std::string          m_Filepath;
    std::string          m_TempDirectory;
    std::string          m_WorkingDirectory;
    std::string          m_ConfigDirectory;

    TentApp*             m_pTentApp; 
    FileManager*         m_pFileManager;
    ConnectionManager*   m_pConnectionManager;

    void (*mCallback)(int, void*);
};

#endif

