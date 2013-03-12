#ifndef PASSPHRASE_H_
#define PASSPHRASE_H_
#pragma once

#include <string>

#include "phrasetoken.h"
#include "credentialsmanager.h"
#include "netlib.h"
#include "entity.h"
#include "masterkey.h"
#include "crypto.h"

namespace pass {

static int RegisterPassphraseWithAttic(const std::string& pass, 
                                       const std::string& masterkey,
                                       PhraseToken& pt,
                                       CredentialsManager* pCm,
                                       Entity& entity);

static int RegisterPassphraseProfilePost(const std::string& encryptedKey, 
                                         const std::string& salt,
                                         CredentialsManager* pCm,
                                         Entity& entity);

static int ConstructMasterKey(const std::string& masterkey, 
                              MasterKey& out,
                              CredentialsManager* pCm,
                              PhraseToken& pt);

static int EncryptKeyWithPassphrase(const std::string& key, 
                                    const std::string& phrasekey, 
                                    const std::string& salt,
                                    std::string& keyOut);

static int RegisterPassphraseWithAttic(const std::string& pass, 
                                       const std::string& masterkey,
                                       PhraseToken& pt,
                                       CredentialsManager* pCm,
                                       Entity& entity)
{
    int status = ret::A_OK;
    // Have the passphrase, and the master key
    // generate random iv and salt
    // Inward facing method
    // Register a new passphrase.

    MasterKey mk;
    status = ConstructMasterKey(masterkey, mk, pCm, pt); // also generates salt, inserts into 
                                                // phrase token

    std::string genmk; mk.GetMasterKey(genmk);

    if(status == ret::A_OK) {
        // Insert sentinel value
        mk.InsertSentinelIntoMasterKey();
        // Get Salt
        std::string salt;
        pt.GetSalt(salt);  // Phrase Token
        // Get Dirty Master Key (un-encrypted master key with sentinel values)
        std::string dirtykey;
        mk.GetMasterKeyWithSentinel(dirtykey);
        pt.SetDirtyKey(dirtykey); // Set Phrase Token
        // Enter passphrase to generate key.
        std::string phraseKey;
        status = pCm->EnterPassphrase(pass, salt, phraseKey);
        if(status == ret::A_OK) {
            pt.SetPhraseKey(phraseKey);
            // Setup passphrase cred to encrypt master key
            std::string encryptedkey;
            status = EncryptKeyWithPassphrase( dirtykey, 
                                               phraseKey, 
                                               salt, 
                                               encryptedkey);
            if(status == ret::A_OK) {
                status = RegisterPassphraseProfilePost( encryptedkey, 
                                                        salt,
                                                        pCm,
                                                        entity);
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
       
    return status; 
}

static int RegisterPassphraseProfilePost(const std::string& encryptedKey, 
                                         const std::string& salt,
                                         CredentialsManager* pCm,
                                         Entity& entity)
{
    int status = ret::A_OK;

    // Create Profile post for 
    AtticProfileInfo* pAtticProf = new AtticProfileInfo();
    // MasterKey with sentinel and Salt
    pAtticProf->SetMasterKey(encryptedKey);
    pAtticProf->SetSalt(salt);

    // Save and post
    std::string output;
    jsn::SerializeObject(pAtticProf, output);

    std::string url;
    entity.GetFrontProfileUrl(url);

    AccessToken at;
    if(pCm) {
        pCm->GetAccessTokenCopy(at);
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    if(status == ret::A_OK) {
        // UrlParams params
        std::string hold(cnst::g_szAtticProfileType);
        hold = netlib::UriEncode(hold);

        url.append("/");
        url.append(hold);

        Response resp;
        netlib::HttpPut(url, NULL, output, &at, resp);

        if(resp.code != 200) {
            status = ret::A_FAIL_NON_200;
            alog::Log(Logger::DEBUG, "RegisterPassphraseProfilePost : \n" +
                      resp.CodeAsString() +
                      resp.body);
        }
    }

    return status;
}

static int ConstructMasterKey(const std::string& masterkey, 
                              MasterKey& out,
                              CredentialsManager* pCm,
                              PhraseToken& pt)
{
    int status = ret::A_OK;
    // Enter passphrase to generate key.
    pCm->RegisterPassphrase(masterkey, pt); // This generates a random salt
                                                         // Sets Phrase key
    pCm->CreateMasterKeyWithPass(out, masterkey); // Create Master Key with given pass
    pCm->SetMasterKey(out);

    return status;
}

static int EncryptKeyWithPassphrase( const std::string& key, 
                                     const std::string& phrasekey, 
                                     const std::string& salt,
                                     std::string& keyOut)
{
    int status = ret::A_OK;

    // encryption credentials
    Credentials enc;
    enc.SetKey(phrasekey);
    enc.SetIv(salt);

    // Encrypt MasterKey with passphrase key
    std::string out;
    crypto::EncryptStringCFB(key, enc, keyOut);
    
    return status;
}

};

#endif

