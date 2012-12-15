
#ifndef CREDENTIALSMANAGER_H_
#define CREDENTIALSMANAGER_H_
#pragma once

#include "masterkey.h"
#include "accesstoken.h"

class CredentialsManager
{
public:
    CredentialsManager();
    ~CredentialsManager();


private:
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts

};



#endif

