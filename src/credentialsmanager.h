
#ifndef CREDENTIALSMANAGER_H_
#define CREDENTIALSMANAGER_H_
#pragma once

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

    MasterKey GetMasterKeyCopy() const      { return m_MasterKey; }
    AccessToken GetAccessTokenCopy() const  { return m_AccessToken; }

    void SetAccessToken(const AccessToken& at)  { m_AccessToken = at; }
    void SetMasterKey(const MasterKey& mk)      { m_MasterKey = mk; }

private:
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts

};



#endif

