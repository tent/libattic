#ifndef LIBATTICUTILS_H_
#define LIBATTICUTILS_H_
#pragma once

#include <string>

#include "utils.h"
#include "constants.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "taskarbiter.h"
#include "taskmanager.h"
#include "filesystem.h"

// Inward facing utility methods used at libattic interface level
namespace liba {
    int InitializeCredentialsManager( CredentialsManager** pCm,
                                      const std::string& configDir)
    {
        int status = ret::A_OK;

        if(!(*pCm))
        {
            (*pCm) = new CredentialsManager();
            (*pCm)->SetConfigDirectory(configDir);
            status = (*pCm)->Initialize();
        }
        else
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;
        return status;
    }

    int InitializeTaskManager(TaskManager** pUm,
                              FileManager* pFm,
                              CredentialsManager* pCm,
                              const AccessToken& at,
                              const Entity& entity,
                              const std::string& tempdir,
                              const std::string& workingdir,
                              const std::string& configdir)
    {
        int status = ret::A_OK;
        if(!pFm) std::cout<<"INVALID FILEMANAGER " << std::endl;
        if(!(*pUm))
        {
            (*pUm) = new TaskManager(pFm,
                                     pCm,
                                     at,
                                     entity,
                                     tempdir,
                                     workingdir,
                                     configdir);

            status = (*pUm)->Initialize();
        }
        else
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;
        return status;
    }

    int InitializeTaskArbiter(const unsigned int threadCount) {
        int status = ret::A_OK;
        status = TaskArbiter::GetInstance()->Initialize(threadCount);
        return status;
    }

    int InitializeTaskFactory( TaskFactory** pTf) {
        int status = ret::A_OK;
        if(!(*pTf))
        {
            (*pTf) = new TaskFactory();
            status = (*pTf)->Initialize();
        }
        else
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;

        return status;
    }

    int ShutdownTaskArbiter() {
        int status = ret::A_OK;
        status = TaskArbiter::GetInstance()->Shutdown();

        return status;
    }

    int ShutdownTaskFactory( TaskFactory* pTf ) {
        int status = ret::A_OK;
        // Blind shutdown
        if(pTf)
        {
            pTf->Shutdown();
            delete pTf;
            pTf = NULL;
        }
        else
            status = ret::A_FAIL_INVALID_PTR;


        return status;
    }

    int ShutdownCredentialsManager(CredentialsManager** pCm) {
        int status = ret::A_OK;
        if((*pCm))
        {
            (*pCm)->Lock();
            status = (*pCm)->Shutdown();
            (*pCm)->Unlock();

            delete (*pCm);
            (*pCm) = NULL;
        }
        else
            status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;


        return status;
    }

    int ShutdownAppInstance(TentApp** pApp)
    {
        int status = ret::A_OK;
        if((*pApp))
        {
            delete (*pApp);
            (*pApp) = NULL;
        }
        else
            status = ret::A_FAIL_INVALID_APP_INSTANCE;


        return status;
    }

    int ShutdownTaskManager( TaskManager** pUm)
    {
        int status = ret::A_OK;
        if((*pUm))
            status = (*pUm)->Shutdown();
        else
            status = ret::A_FAIL_INVALID_UPLOADMANAGER_INSTANCE;


        return status;
    }

    int DeserializeIntoAccessToken(const std::string& body, AccessToken& out)
    {
        int status = ret::A_OK;
    
        if(!jsn::DeserializeObject(&out, body))
            status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          

        return status;
    }

    int WriteOutAccessToken(AccessToken& at, const std::string& outDir)
    {
        int status = ret::A_OK;
        std::string path = outDir;
        utils::CheckUrlAndAppendTrailingSlash(path);      
        path.append(cnst::g_szAuthTokenName);
        std::cout<<" OUT PATH : " << path << std::endl;
        status = at.SaveToFile(path);
        return status;
    }


}

#endif

