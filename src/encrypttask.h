
#ifndef ENCRYPTTASK_H_
#define ENCRYPTTASK_H_
#pragma once

#include <string>
#include "cryptotask.h"

class EncryptTask : public CryptoTask
{

public:
    EncryptTask( const std::string& filepath, const std::string& outpath);

    ~EncryptTask();

    void RunTask();

private:
    std::string     m_Filepath;
    std::string     m_Outpath;


};

#endif

