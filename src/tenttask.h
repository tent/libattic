
#ifndef TENTTASK_H_
#define TENTTASK_H_
#pragma once

#include <string>

#include "tentapp.h"
#include "task.h"

#include "filemanager.h"
#include "connectionmanager.h"


// TODO :: WIP tent specific task abstractiom, abstract all things common
//          out of push pull and other tent tasks here. do this after other tasks work 100%
class TentTask : public Task
{

public:
    TentTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir)
    {
        m_pTentApp = pApp;
        m_pFileManager = pFm;
        m_pConnectionManager = pCon; 

        m_At = at;

        m_Entity = entity;
        m_Filepath = filepath;
        m_TempDirectory = tempdir;
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
    
    void GetEntity(std::string &out)        { out = m_Entity; }
    void GetFilepath(std::string &out)      { out = m_Filepath; }
    void GetTempDirectory(std::string &out) { out = m_TempDirectory; } 

    TentApp* GetTentApp()                       { return m_pTentApp; }
    FileManager* GetFileManager()               { return m_pFileManager; } 
    ConnectionManager* GetConnectionManager()   { return m_pConnectionManager; } 

    void SetCallback(void (*cb)(int, void*)) { callback = cb; }

protected:
    void Callback()
    {
        std::cout<<" Callback " << std::endl;
        if(callback)
            callback(0, NULL); 

    }
private:
//protected:
    AccessToken          m_At;

    std::string          m_Entity;
    std::string          m_Filepath;
    std::string          m_TempDirectory;

    TentApp*             m_pTentApp; 
    FileManager*         m_pFileManager;
    ConnectionManager*   m_pConnectionManager;

    void (*callback)(int, void*);
};

#endif

