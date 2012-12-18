

#ifndef SYNCMANIFESTTASK_H_
#define SYNCMANIFESTTASK_H_
#pragma once

#include "tenttask.h"
#include "metastorepost.h"
#include "crypto.h"

class SyncManifestTask : public TentTask
{
    
    FileInfo* CreateManifestFileInfoAndEncrypt();
    void GetManifestPostID(std::string& out);
    int PullManifestPostAttachment(const std::string& postid);
    int GetFileAndWriteOut(const std::string& url, const std::string &filepath);
    int SearchForManifestPost(MetaStorePost& out);
    void CreateManifestPost(MetaStorePost& post);
    int PushManifestPost(const std::string& postid, MetaStorePost* post);

    int DeleteManifestPost(const std::string& postid);

    int PostManifest(const std::string& url, const std::string& filepath);
    int PutManifest(const std::string& url, const std::string& filepath);

    void GetMasterKeyFromCredentials(MasterKey& mk, std::string& outpath);

public:
    SyncManifestTask( TentApp* pApp, 
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

    ~SyncManifestTask();
    
    void RunTask();

private:
    Crypto  m_Crypto;
    MetaStorePost m_ManifestPost;
 
};


#endif

