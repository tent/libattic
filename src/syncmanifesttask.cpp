
#include "syncmanifesttask.h"

SyncManifestTask::SyncManifestTask( TentApp* pApp, 
                  FileManager* pFm, 
                  ConnectionManager* pCon, 
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
                            at,
                            entity,
                            filepath,
                            tempdir,
                            workingdir,
                            configdir,
                            callback)
{

}

SyncManifest::~SyncManifestTask()
{

}

void SyncManifest::RunTask()
{

}



