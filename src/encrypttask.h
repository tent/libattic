
#ifndef ENCRYPTTASK_H_
#define ENCRYPTTASK_H_
#pragma once


#include "tenttask.h"

class EncryptTask : public TentTask
{

public:
    EncryptTask( TentApp* pApp, 
                 FileManager* pFm, 
                 ConnectionManager* pCon, 
                 CredentialsManager* pCm,
                 const AccessToken& at,
                 const std::string& entity,
                 const std::string& filepath,
                 const std::string& tempdir,
                 const std::string& workingdir,
                 const std::string& configdir,
                 void (*callback)(int, void*));

    ~EncryptTask();

    void RunTask();



};

#endif

