#include "encrypttask.h"

EncryptTask::EncryptTask( const std::string& filepath, 
                          const std::string& outpath, 
                          const Credentials* pCred, 
                          bool generate)

{
    m_Filepath = filepath;
    m_Outpath = outpath;

    if(pCred)
    {
        // Copy credentials
        m_Cred = *pCred;
    }

    m_Generate = generate;

}

EncryptTask::~EncryptTask()
{

}

void EncryptTask::RunTask()
{
    int status = EncryptFile(m_Filepath, m_Outpath);

    Callback(status, NULL);
}

int EncryptTask::EncryptFile(const std::string& filepath, const std::string& outpath)
{
    if(m_Generate)
    {
        // TODO :: change this to pass by ref
        m_Cred = GetCrypto()->GenerateCredentials();
    }

    int status = GetCrypto()->EncryptFile(filepath, outpath, m_Cred);

    return status;
}

