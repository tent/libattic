
#ifndef CREDENTIALSMANAGER_H_
#define CREDENTIALSMANAGER_H_
#pragma once

#include <string>

#include "mutexclass.h"
#include "taskarbiter.h"
#include "masterkey.h"
#include "accesstoken.h"
#include "phrasetoken.h"
#include "crypto.h"

class CredentialsManager : public MutexClass
{
    int GenerateMasterKey();
    void ConstructPhraseTokenPath(std::string& out);
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

    // Phrase Token
    int DeserializeIntoPhraseToken(const std::string& buffer);
    int WriteOutPhraseToken();
    int LoadPhraseToken();

    // Pass Phrase
    int EnterPassphrase( const std::string& pass, 
                         std::string& salt, 
                         PhraseToken& ptOut);

    int RegisterPassphrase( const std::string& pass, 
                            PhraseToken& ptOut);

    int ChangePassphrase( const std::string& oldpass, 
                          const std::string& newpass, 
                          PhraseToken& ptOut);
    
    int GenerateMasterKey( MasterKey& mkOut);

    // MasterKey

    //int EnterUserNameAndPassword(const std::string& user, const std::string& pass);
    void GetManifestPath(std::string& out)      { ConstructManifestPath(out); }
    void GetAccessTokenPath(std::string& out)   { ConstructAccessTokenPath(out); }
    void GetMasterKeyCopy(MasterKey& key)       { key = m_MasterKey; }
    void GetAccessTokenCopy(AccessToken& tk)    { tk = m_AccessToken; }

    void SetConfigDirectory(const std::string& dir) { m_ConfigDirectory = dir; }
    void SetAccessToken(const AccessToken& at)      { m_AccessToken = at; }
    void SetMasterKey(const MasterKey& mk)          { m_MasterKey = mk; }

private:
    Crypto          m_Crypto;
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts
    PhraseToken     m_PhraseToken;

    std::string     m_ConfigDirectory;
};



#endif

