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

namespace attic { 

// Make sure this is the single source of truth for the AccessToken
// the time offset will be set here and needs to propagate outward
class CredentialsManager {
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
    int GenerateMasterKey( std::string& keyOut);
    void GenerateMasterKey( MasterKey& mkOut);
    void CreateMasterKeyWithPass(const std::string& key, MasterKey& mkOut);
    void GeneratePublicPrivateKeyPair(std::string& public_out, std::string& private_out);

    // MasterKey
    void GetManifestPath(std::string& out)      { ConstructManifestPath(out); }
    void GetAccessTokenPath(std::string& out)   { ConstructAccessTokenPath(out); }

    // Public/Private Key
    void GetPrivateKey(std::string& out) {
        sk_mtx_.Lock();
        out.append(private_key_.c_str(), private_key_.size());
        sk_mtx_.Unlock();
    }

    void GetPublicKey(std::string& out) {
        pk_mtx_.Lock();
        out.append(public_key_.c_str(), public_key_.size());
        pk_mtx_.Unlock();
    }
    
    void GetMasterKeyCopy(MasterKey& key) {
        mk_mtx_.Lock();
        key = master_key_;
        mk_mtx_.Unlock();
    }

    void GetAccessTokenCopy(AccessToken& tk) {
        at_mtx_.Lock(); 
        tk = access_token_; 
        at_mtx_.Unlock();
    }

    void SetConfigDirectory(const std::string& dir) {
        config_directory_ = dir; 
    }

    void SetAccessToken(const AccessToken& at) {
        at_mtx_.Lock(); 
        access_token_ = at; 
        at_mtx_.Unlock();
    }

    void SetMasterKey(const MasterKey& mk) {
        mk_mtx_.Lock();
        master_key_ = mk; 
        mk_mtx_.Unlock();
    }

    void set_master_key(const std::string& masterkey) {
        MasterKey mk;
        mk.SetMasterKey(masterkey);
        SetMasterKey(mk);
    }

    void set_time_offset(const long int offset) {
        at_mtx_.Lock();
        access_token_.set_time_offset(offset);
        at_mtx_.Unlock();
    }

    void set_public_key(const std::string& key) {
        pk_mtx_.Lock();
        public_key_ = key;
        pk_mtx_.Unlock();
    }

    void set_private_key(const std::string& key) {
        sk_mtx_.Lock();
        private_key_ = key;
        sk_mtx_.Unlock();

    }

private:
    MutexClass      mk_mtx_;
    MasterKey       master_key_;    // Master Key used to encrypt sqlitedb

    MutexClass      at_mtx_;
    AccessToken     access_token_;  // Access Token used to auth during tent posts

    MutexClass      pt_mtx_;
    PhraseToken     phrase_token_;

    std::string     config_directory_;

    MutexClass      pk_mtx_;
    std::string     public_key_;
    MutexClass      sk_mtx_;
    std::string     private_key_;
};

} //namespace
#endif

