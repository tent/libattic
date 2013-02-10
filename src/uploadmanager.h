#ifndef UPLOADMANAGER_H_
#define UPLOADMANAGER_H_
#pragma once

#include <string>

#include "accesstoken.h"
#include "entity.h"
#include "taskfactory.h"

class TentApp;
class FileManager;
class CredentialsManager;

class UploadManager : public TaskFactoryDelegate
{
public:
    UploadManager( TentApp* pApp, 
                   FileManager* pFm, 
                   CredentialsManager* pCm,
                   const AccessToken& at,
                   const Entity& entity,
                   const std::string& tempdir, 
                   const std::string& workingdir,
                   const std::string& configdir
                 );

    ~UploadManager();

    int Initialize();
    int Shutdown();

    virtual void OnTaskCreate(Task* t);
    virtual void OnTaskInsert(Task* t);

    int UploadFile(const std::string& filepath, void (*callback)(int, void*));
    int DownloadFile(const std::string& filepath, void (*callback)(int, void*));
    int DownloadAllFiles(void (*callback)(int, void*));
    int SyncFiles(void (*callback)(int, void*));
    int DeleteFile(const std::string& filepath, void (*callback)(int, void*));

private:
    TaskFactory             m_TaskFactory; // Local to upload manager

    TentApp*                m_pApp;
    FileManager*            m_pFileManager;
    CredentialsManager*     m_pCredentialsManager;

    AccessToken             m_AccessToken;
    Entity                  m_Entity;

    std::string             m_TempDir;
    std::string             m_WorkingDir;
    std::string             m_ConfigDir;
};

#endif

