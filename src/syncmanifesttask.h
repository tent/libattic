

#ifndef SYNCMANIFESTTASK_H_
#define SYNCMANIFESTTASK_H_
#pragma once

#include "tenttask.h"
#include "metastorepost.h"

class SyncManifestTask : public TentTask
{
    
    void GetManifestPostID(std::string& out);
    void PullManifestPost(const std::string id);
    int SearchForManifestPost(MetaStorePost& out);
    void CreateManifestPost(MetaStorePost& post);
    void PushManifestPost(const std::string& postID, MetaStorePost* post);

public:
    SyncManifestTask( TentApp* pApp, 
                      FileManager* pFm, 
                      ConnectionManager* pCon, 
                      const AccessToken& at,
                      const std::string& entity,
                      const std::string& filepath,
                      const std::string& tempdir, 
                      const std::string& workingdir,
                      const std::string& configdir,
                      void (*callback)(int, void*));

    ~SyncManifestTask();
    
    void RunTask();
 
};


#endif

