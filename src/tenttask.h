#ifndef TENTTASK_H_
#define TENTTASK_H_
#pragma once

#include <string>

#include "tentapp.h"
#include "task.h"

#include "entity.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskarbiter.h"
#include "taskfactory.h"

class TentTask : public Task
{
public:
    TentTask( Task::TaskType type,
              TentApp* pApp, 
              FileManager* pFm, 
              CredentialsManager* pCm,
              TaskArbiter* pTa,
              TaskFactory* pTf,
              const AccessToken& at,
              const Entity& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*))
              : 
              Task(type)
    {
        m_pTentApp              = pApp;
        m_pFileManager          = pFm;
        m_pCredentialsManager   = pCm;
        m_pTaskArbiter          = pTa;
        m_pTaskFactory          = pTf;

        m_At = at;

        m_Entity = entity;
        m_Filepath = filepath;
        m_TempDirectory = tempdir;
        m_WorkingDirectory = workingdir;
        m_ConfigDirectory = configdir;

        mCallback = callback;
    }

    virtual void SetFinishedState()                                         
    {                                                                       
        Task::SetFinishedState();
        if(mCallback)                                              
        {
            mCallback(ret::A_OK, NULL);                            
            mCallback = NULL; // Invalidate functor
        }
    }                                                                       

    virtual ~TentTask()
    {
        m_pTentApp              = NULL;
        m_pFileManager          = NULL;
        m_pCredentialsManager   = NULL;
        m_pTaskArbiter          = NULL;
        m_pTaskFactory          = NULL;
    }

    /*
    virtual void RunTask() {}
    */

    AccessToken* GetAccessToken()       { return &m_At; }
    AccessToken  GetAccessTokenCopy()   { return m_At; }
    
    void GetEntityUrl(std::string& out)         { m_Entity.GetEntityUrl(out); }
    void GetEntity(Entity& out)                 { out = m_Entity; }
    void GetFilepath(std::string& out)          { out = m_Filepath; }
    void GetTempDirectory(std::string& out)     { out = m_TempDirectory; } 
    void GetWorkingDirectory(std::string& out)  { out = m_WorkingDirectory; }
    void GetConfigDirectory(std::string& out)   { out = m_ConfigDirectory; }

    TentApp*            GetTentApp()            { return m_pTentApp; }
    FileManager*        GetFileManager()        { return m_pFileManager; } 
    CredentialsManager* GetCredentialsManager() { return m_pCredentialsManager; } 
    TaskArbiter*        GetTaskArbiter()        { return m_pTaskArbiter; } 
    TaskFactory*        GetTaskFactory()        { return m_pTaskFactory; }

    void SetTentApp(TentApp* pApp)                        { m_pTentApp = pApp; }
    void SetFileManager(FileManager* pFm)                 { m_pFileManager = pFm; }
    void SetCredentialsManager(CredentialsManager* pCm)   { m_pCredentialsManager = pCm; }
    void SetTaskArbiter(TaskArbiter* pTa)                 { m_pTaskArbiter = pTa; }
    void SetTaskFactory(TaskFactory* pTf)                 { m_pTaskFactory = pTf; }

    void SetAccessToken(const AccessToken& at)                  { m_At = at; }
    void SetEntity(const Entity& entity)                        { m_Entity = entity; }
    void SetFilepath(const std::string& filepath)               { m_Filepath = filepath; }
    void SetTempDirectory(const std::string& tempdir)           { m_TempDirectory = tempdir; }
    void SetWorkingDirectory(const std::string& workingdir)     { m_WorkingDirectory = workingdir; }
    void SetConfigDirectory(const std::string& configdir)       { m_ConfigDirectory = configdir; }
    void SetCallback(void (*cb)(int, void*))                    { mCallback = cb; }


    void Reset()
    {
        m_pTentApp              = NULL;
        m_pFileManager          = NULL;
        m_pCredentialsManager   = NULL;
        m_pTaskArbiter          = NULL;
        m_pTaskFactory          = NULL;

        m_At.Reset();
        m_Entity.Reset();

        m_Filepath.clear();
        m_TempDirectory.clear();
        m_WorkingDirectory.clear();
        m_ConfigDirectory.clear();

        mCallback = NULL;
    }

protected:
    void Callback(int code, void* p)
    {
        if(mCallback)
            mCallback(code, p); 
    }

    void ConstructPostUrl(std::string& out)
    {
        Entity entity;
        GetEntity(entity);
        entity.GetApiRoot(out);
        out += "/posts";
    }

private:
    AccessToken          m_At;
    Entity               m_Entity;

    std::string          m_Filepath;
    std::string          m_TempDirectory;
    std::string          m_WorkingDirectory;
    std::string          m_ConfigDirectory;

    // Shared ptrs,
    TentApp*             m_pTentApp;  // TODO :: take a look at if this is actually being used
    FileManager*         m_pFileManager;
    CredentialsManager*  m_pCredentialsManager;
    TaskArbiter*         m_pTaskArbiter;
    TaskFactory*         m_pTaskFactory;

    void (*mCallback)(int, void*);
};

#endif

