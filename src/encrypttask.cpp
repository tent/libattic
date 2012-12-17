#include "encrypttask.h"

EncryptTask::EncryptTask( TentApp* pApp, 
                          FileManager* pFm, 
                          ConnectionManager* pCon, 
                          CredentialsManager* pCm,
                          const AccessToken& at,
                          const std::string& entity,
                          const std::string& filepath,
                          const std::string& tempdir,
                          const std::string& workingdir,
                          const std::string& configdir,
                          void (*callback)(int, void*))
                         :
                         TentTask( pApp,
                                   pFm,
                                   pCon,
                                   pCm,
                                   at,
                                   entity,
                                   filepath,
                                   tempdir,
                                   workingdir,
                                   configdir,
                                   callback )
{


}

EncryptTask::~EncryptTask()
{

}

void EncryptTask::RunTask()
{

}



