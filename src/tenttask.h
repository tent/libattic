#ifndef TENTTASK_H_
#define TENTTASK_H_
#pragma once

#include <string>

#include "task.h"
#include "entity.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskdelegate.h"

class TentTask : public Task {
public:
    TentTask(Task::TaskType type,
             FileManager* pFm, 
             CredentialsManager* pCm,
             const AccessToken& at,
             const Entity& entity,
             const std::string& filepath,
             const std::string& tempdir, 
             const std::string& workingdir,
             const std::string& configdir,
             TaskDelegate* callbackDelegate = NULL)
             : 
             Task(type)
    {
        m_pFileManager          = pFm;
        m_pCredentialsManager   = pCm;

        m_At = at;

        m_Entity = entity;
        m_Filepath = filepath;
        m_TempDirectory = tempdir;
        m_WorkingDirectory = workingdir;
        m_ConfigDirectory = configdir;

        m_pCallbackDelegate = callbackDelegate;
    }

    virtual void SetFinishedState() {
        Task::SetFinishedState();
    }                                                                       

    virtual ~TentTask() {
        m_pFileManager          = NULL;
        m_pCredentialsManager   = NULL;
        m_pCallbackDelegate     = NULL;
    }

    /*
    virtual void RunTask() {}
    */

    AccessToken* GetAccessToken()       { return &m_At; }
    AccessToken  GetAccessTokenCopy()   { return m_At; }
    
    void GetEntityUrl(std::string& out)         { out = m_Entity.entity(); } // refactor this to conform with const string & V03
    void GetEntity(Entity& out)                 { out = m_Entity; }
    void GetFilepath(std::string& out)          { out = m_Filepath; }
    void GetTempDirectory(std::string& out)     { out = m_TempDirectory; } 
    void GetWorkingDirectory(std::string& out)  { out = m_WorkingDirectory; }
    void GetConfigDirectory(std::string& out)   { out = m_ConfigDirectory; }

    FileManager*        GetFileManager()        { return m_pFileManager; } 
    CredentialsManager* GetCredentialsManager() { return m_pCredentialsManager; } 

    void SetFileManager(FileManager* pFm)                 { m_pFileManager = pFm; }
    void SetCredentialsManager(CredentialsManager* pCm)   { m_pCredentialsManager = pCm; }

    void SetAccessToken(const AccessToken& at)                  { m_At = at; }
    void SetEntity(const Entity& entity)                        { m_Entity = entity; }
    void SetFilepath(const std::string& filepath)               { m_Filepath = filepath; }
    void SetTempDirectory(const std::string& tempdir)           { m_TempDirectory = tempdir; }
    void SetWorkingDirectory(const std::string& workingdir)     { m_WorkingDirectory = workingdir; }
    void SetConfigDirectory(const std::string& configdir)       { m_ConfigDirectory = configdir; }

protected:
    void Callback(const int code, const std::string& var) {
        if(m_pCallbackDelegate)
            m_pCallbackDelegate->Callback(GetTaskType(), code, GetTaskState(), var);
    }

    void GetApiRoot(std::string& out) {
        Entity entity;
        GetEntity(entity);
        //entity.GetApiRoot(out); // UPDATE THIS V03
    }

    void ConstructPostUrl(std::string& out) {
        GetApiRoot(out);
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
    FileManager*         m_pFileManager;
    CredentialsManager*  m_pCredentialsManager;

    TaskDelegate* m_pCallbackDelegate;
};

#endif

