
#ifndef LIBATTICUTILS_H_
#define LIBATTICUTILS_H_
#pragma once

#include <string>
#include <iostream>

#include "utils.h"

#include "entitymanager.h"
#include "connectionmanager.h"
#include "credentialsmanager.h"
#include "filemanager.h"
//#include "entity.h"

// Inward facing utility methods used at libattic interface level

/*
int InitializeFileManager();
int InitializeCredentialsManager();
int InitializeEntityManager();

int ShutdownFileManager();
int ShutdownCredentialsManager();
int ShutdownAppInstance();
int ShutdownEntityManager();
*/
/*
int SetWorkingDirectory(const char* szDir);
int SetConfigDirectory(const char* szDir);
int SetTempDirectory(const char* szDir);

int LoadEntity();
int SaveEntity();

int LoadPhraseToken();
int SavePhraseToken(PhraseToken& pt);

int LoadMasterKey(); // call with a valid phrase token

void GetPhraseTokenFilepath(std::string& out);
void GetEntityFilepath(std::string& out);

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey);

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv);
*/


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
            std::cout<<" creating cred manager" << std::endl;
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
}

#endif

