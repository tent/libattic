
#ifndef CREDENTIALSMANAGER_H_
#define CREDENTIALSMANAGER_H_
#pragma once

#include <string>

#include "taskarbiter.h"
#include "masterkey.h"
#include "accesstoken.h"
#include "crypto.h"


class CredentialsManager : public MutexClass
{

    void ConstructAccessTokenPath(std::string& out);
    void ConstructManifestPath(std::string& out);
public:
    CredentialsManager();
    ~CredentialsManager();

    int Initialize();
    int Shutdown();

    // Access Token 
    int DeserializeIntoAccessToken(const std::string& buffer); 
    int WriteOutAccessToken();
    int LoadAccessToken();

    // MasterKey
    int EnterUserNameAndPassword(const std::string& user, const std::string& pass);


    void GetManifestPath(std::string& out);
    void GetAccessTokenPath(std::string& out);
    void GetMasterKeyCopy(MasterKey& key);
    void GetAccessTokenCopy(AccessToken& tk);

    void SetConfigDirectory(const std::string& dir);
    void SetAccessToken(const AccessToken& at);     
    void SetMasterKey(const MasterKey& mk);     

private:
    Crypto          m_Crypto;
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts

    std::string     m_ConfigDirectory;
};



#endif

