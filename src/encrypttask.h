
#ifndef ENCRYPTTASK_H_
#define ENCRYPTTASK_H_
#pragma once

#include <string>
#include "cryptotask.h"


class EncryptTask : public CryptoTask
{
    int EncryptFile(const std::string& filepath, const std::string& outpath);

public:
    EncryptTask( const std::string& filepath, 
                 const std::string& outpath, 
                 const Credentials* pCred = NULL, 
                 bool generate=true);

    ~EncryptTask();

    void RunTask();

private:
    Credentials     m_Cred;

    std::string     m_Filepath;
    std::string     m_Outpath;

    bool            m_Generate; // whether or not to generate credentials

};

#endif

