
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

SyncManifestTask::~SyncManifestTask()
{

}

void SyncManifestTask::RunTask()
{
    // Get Metadata Post id
        // If none exists create first metadata post
    // Pull Metadata Post
        // Compare versions
        // If server version newer, replace client version
            // This is more involved, if manifest is direct there needs to be some sort of merge
        // If client version is newer, PUT new post, (bump version number) 
}



