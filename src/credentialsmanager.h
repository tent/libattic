
#ifndef CREDENTIALSMANAGER_H_
#define CREDENTIALSMANAGER_H_
#pragma once

#include <string>

#include "taskarbiter.h"
#include "masterkey.h"
#include "accesstoken.h"


class CredentialsManager : public MutexClass
{

public:
    CredentialsManager();
    ~CredentialsManager();

    int Initialize();
    int Shutdown();

    // Access Token 
    int DeserializeIntoAccessToken(const std::string& buffer); 
    int WriteOutAccessToken();
    int LoadAccessToken();

    void ConstructAccessTokenPath(std::string& out);
    void ConstructManifestPath(std::string& out);

    MasterKey GetMasterKeyCopy() const      { return m_MasterKey; }
    AccessToken GetAccessTokenCopy() const  { return m_AccessToken; }

    void SetConfigDirectory(const std::string& dir)     { m_ConfigDirectory = dir; }
    void SetAccessToken(const AccessToken& at)          { m_AccessToken = at; }
    void SetMasterKey(const MasterKey& mk)              { m_MasterKey = mk; }

private:
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts

    std::string     m_ConfigDirectory;
};



#endif

