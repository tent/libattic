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


// TODO :: the necessity of this whole class needs to be re-thought,
//          alot of generic methods are here that can be abstracted to 
//          a more functional namespace, reducing locks.
class CredentialsManager : public MutexClass {
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
                         std::string& keyOut);

    int RegisterPassphrase( const std::string& pass, 
                            PhraseToken& ptOut);

    int ChangePassphrase( const std::string& oldpass, 
                          const std::string& newpass, 
                          PhraseToken& ptOut);
    
    int GenerateMasterKey( std::string& keyOut);
    void GenerateMasterKey( MasterKey& mkOut);
    void CreateMasterKeyWithPass(const std::string& key, MasterKey& mkOut);

    // MasterKey
    void GetManifestPath(std::string& out)      { ConstructManifestPath(out); }
    void GetAccessTokenPath(std::string& out)   { ConstructAccessTokenPath(out); }
    void GetMasterKeyCopy(MasterKey& key) {
        Lock();
        key = m_MasterKey; 
        Unlock();
    }
    void GetAccessTokenCopy(AccessToken& tk) {
        Lock(); 
        tk = m_AccessToken; 
        Unlock();
    }

    void SetConfigDirectory(const std::string& dir) {
        Lock();
        m_ConfigDirectory = dir; 
        Unlock();
    }

    void SetAccessToken(const AccessToken& at) {
        Lock(); 
        m_AccessToken = at; 
        Unlock();
    }

    void SetMasterKey(const MasterKey& mk) {
        Lock();
        m_MasterKey = mk; 
        Unlock();
    }

private:
    MasterKey       m_MasterKey;    // Master Key used to encrypt sqlitedb
    AccessToken     m_AccessToken;  // Access Token used to auth during tent posts
    PhraseToken     m_PhraseToken;

    std::string     m_ConfigDirectory;
};
#endif

