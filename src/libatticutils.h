
#ifndef LIBATTICUTILS_H_
#define LIBATTICUTILS_H_
#pragma once

#include <string>

#include "utils.h"

#include "constants.h"
#include "entitymanager.h"
#include "connectionmanager.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "taskarbiter.h"
//#include "entity.h"

// Inward facing utility methods used at libattic interface level

namespace liba
{
    int InitializeFileManager( FileManager** pFm, 
                               const std::string& manifestName,
                               const std::string& configDir, 
                               const std::string& tempDir)
    {
        int status = ret::A_OK;

        if(!(*pFm))
        {
            // Construct path
            std::string filepath(configDir);
            utils::CheckUrlAndAppendTrailingSlash(filepath);
            filepath.append(manifestName);

                (*pFm) = new FileManager(filepath, configDir);
                (*pFm)->SetTempDirectory(tempDir);

                if(!(*pFm)->StartupFileManager())
                {
                    status = ret::A_FAIL_TO_LOAD_FILE;
                }
        }
        else
        {
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;
        }

        return status;
    }

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
        {
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;
        }

        return status;
    }

    int InitializeEntityManager( EntityManager** pEm )
    {
        int status = ret::A_OK;
        if(!(*pEm))
        {
            (*pEm) = new EntityManager();
            status = (*pEm)->Initialize();
        }
        else
        {
            status = ret::A_FAIL_ATTEMPT_TO_REINIT;
        }

        return status;
    }

    int InitializeTaskArbiter(const unsigned int threadCount)
    {
        int status = ret::A_OK;
        status = TaskArbiter::GetInstance()->Initialize(threadCount);
        return status;
    }

    int ShutdownTaskArbiter()
    {
        int status = ret::A_OK;
        status = TaskArbiter::GetInstance()->Shutdown();
        return status;
    }

    int ShutdownFileManager( FileManager* pFm )
    {
        int status = ret::A_OK;
        // Blind shutdown
        if(pFm)
        {
            pFm->ShutdownFileManager();
            delete pFm;
            pFm = NULL;
        }
        else
        {
            status = ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE;
        }

        return status;
    }

    int ShutdownCredentialsManager(CredentialsManager* pCm)
    {
        int status = ret::A_OK;
        if(pCm)
        {
            while(pCm->TryLock()) { sleep(0); }
            status = pCm->Shutdown();
            pCm->Unlock();

            delete pCm;
            pCm = NULL;
        }
        else
        {
            status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
        }

        return status;
    }

    int ShutdownEntityManager(EntityManager* pEm)
    {
        int status = ret::A_OK;
        if(pEm)
        {
            while(pEm->TryLock()) { sleep(0); }
            status = pEm->Shutdown();
            pEm->Unlock();

            delete pEm;
            pEm = NULL;
        }
        else
        {
            status = ret::A_FAIL_INVALID_ENTITYMANAGER_INSTANCE;
        }

        return status;
    }

    int ShutdownAppInstance(TentApp* pApp)
    {
        int status = ret::A_OK;
        if(pApp)
        {
            delete pApp;
            pApp = NULL;
        }
        else
        {
            status = ret::A_FAIL_INVALID_APP_INSTANCE;
        }
        
        return status;
    }

    int ShutdownConnectionManager(ConnectionManager* pCm)
    {

        int status = ret::A_OK;
        if(pCm)
        {
            status = pCm->Shutdown();
            // No need to delete, connection manager takes care of this
            // TODO :: reconsider the use of a singleton with the connection manager,
            // inconsistent design
        }
        else
        {
            status = ret::A_FAIL_INVALID_CONNECTIONMANAGER_INSTANCE;
        }

        return status;
    }

    int DeserializeIntoAccessToken(const std::string& body, AccessToken& out)
    {
        int status = ret::A_OK;
    
        if(!JsonSerializer::DeserializeObject(&out, body))
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

