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
                                       CredentialsManager* pCm,
                                       Entity& entity,
                                       PhraseToken& pt,
                                       std::string& recoverykey);

static int RegisterPassphraseProfilePost(const std::string& encryptedKey, 
                                         const std::string& salt,
                                         CredentialsManager* pCm,
                                         AtticProfileInfo& atticProfile,
                                         Entity& entity);

static int EnterPassphrase(const std::string& pass, 
                           std::string& salt, 
                           std::string& keyOut);

static int EnterRecoveryKey(const std::string& recoverykey,
                            CredentialsManager* pCm,
                            Entity& entity,
                            std::string& keyOut);

static int DecryptKey(const std::string& encryptedkey, 
                      const std::string& phrasekey, 
                      const std::string& iv, 
                      std::string& out);

static int GenerateRecoveryKey(const std::string& masterkey,
                               CredentialsManager* pCm,
                               AtticProfileInfo& atticProfile,
                               Entity& entity,
                               std::string& recoveryOut);

static int ConstructMasterKey(const std::string& masterkey, 
                              CredentialsManager* pCm,
                              PhraseToken& pt,
                              MasterKey& out);

static int EncryptKeyWithPassphrase(const std::string& key, 
                                    const std::string& phrasekey, 
                                    const std::string& salt,
                                    std::string& keyOut);

static int RegisterPassphraseWithAttic(const std::string& pass, 
                                       const std::string& masterkey,
                                       CredentialsManager* pCm,
                                       Entity& entity,
                                       PhraseToken& pt,
                                       std::string& recoverykey)
{
    int status = ret::A_OK;
    // Have the passphrase, and the master key
    // generate random iv and salt
    // Inward facing method
    // Register a new passphrase.
    //

    MasterKey mk;
    status = ConstructMasterKey(masterkey, pCm, pt, mk); // also generates salt, inserts into 
                                                // phrase token
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
                // MasterKey with sentinel and Salt
                AtticProfileInfo atticProfile;
                atticProfile.SetMasterKey(encryptedkey);
                atticProfile.SetSalt(salt);

                std::string genmk; mk.GetMasterKey(genmk);
                status = GenerateRecoveryKey(genmk,
                                             pCm,
                                             atticProfile,
                                             entity,
                                             recoverykey);
                if(status == ret::A_OK) {
                    status = RegisterPassphraseProfilePost(encryptedkey, 
                                                           salt,
                                                           pCm,
                                                           atticProfile,
                                                           entity);
                }
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
       
    return status; 
}

static int GenerateRecoveryKey(const std::string& masterkey,
                               CredentialsManager* pCm,
                               AtticProfileInfo& atticProfile,
                               Entity& entity,
                               std::string& recoveryOut)
{
    int status = ret::A_OK;
    std::string recovery_key;
    utils::GenerateRandomString(recovery_key, 32);

    MasterKey backupkey;
    PhraseToken authcodetoken;
    ConstructMasterKey(masterkey, pCm, authcodetoken, backupkey);

    // Insert sentinel value
    backupkey.InsertSentinelIntoMasterKey();
    // Get Salt
    std::string salt;

    authcodetoken.GetSalt(salt);  // Phrase Token
    // Get Dirty Master Key (un-encrypted master key with sentinel values)
    std::string dirtykey;
    backupkey.GetMasterKeyWithSentinel(dirtykey);
    authcodetoken.SetDirtyKey(dirtykey); // Set Phrase Token
    // Enter passphrase to generate key.
    std::string phraseKey;
    status = pCm->EnterPassphrase(recovery_key, salt, phraseKey);
    if(status == ret::A_OK) {
        authcodetoken.SetPhraseKey(phraseKey);
        // Setup passphrase cred to encrypt master key
        std::string encryptedkey;
        status = EncryptKeyWithPassphrase( dirtykey, 
                                           phraseKey, 
                                           salt, 
                                           encryptedkey);
        if(status == ret::A_OK) {
            atticProfile.SetRecoveryMasterKey(encryptedkey);
            atticProfile.SetRecoverySalt(salt);

            crypto::Base32EncodeString(recovery_key, recoveryOut);
        }
    }

    return status;
}

static int EnterRecoveryKey(const std::string& recoverykey,
                            CredentialsManager* pCm,
                            Entity& entity,
                            std::string& keyOut)
{
    int status = ret::A_OK;
    AtticProfileInfo* atticProfile = entity.GetAtticProfile();
    if(atticProfile) {
        std::string encrypted_masterkey, recovery_salt;
        atticProfile->GetRecoveryMasterKey(encrypted_masterkey);
        atticProfile->GetRecoverySalt(recovery_salt);


        std::string unencoded_recovery_key;
        crypto::Base32DecodeString(recoverykey, unencoded_recovery_key);
        std::string phrasekey;

        EnterPassphrase(unencoded_recovery_key, recovery_salt, phrasekey);


        if(!phrasekey.empty()) {

            std::string decrypted_masterkey;
            status = DecryptKey(encrypted_masterkey, phrasekey, recovery_salt, decrypted_masterkey);
        }
        
    }

    return status;
}

static int DecryptKey(const std::string& encryptedkey, 
                      const std::string& phrasekey, 
                      const std::string& iv, 
                      std::string& out)
{
    int status = ret::A_OK;

    Credentials cred;
    cred.SetKey(phrasekey);
    cred.SetIv(iv);

    std::string decrypted_key;
    status = crypto::DecryptStringCFB(encryptedkey, cred, decrypted_key);

    if(status == ret::A_OK) {
        if(decrypted_key.size() >= 8) {
            // Check sentinel bytes
            std::string sentone, senttwo;
            sentone = decrypted_key.substr(0, 4);
            senttwo = decrypted_key.substr(4, 4);

            if(sentone == senttwo) {
                out = decrypted_key.substr(8);
            }
            else {
                status = ret::A_FAIL_SENTINEL_MISMATCH;
            }
        }
    }

    return status;
}

static int GenerateRecoveryQuestionsKey(const std::string& masterkey,
                                        CredentialsManager* pCm,
                                        Entity& entity,
                                        PhraseToken& pt)
{
    int status = ret::A_OK;

    return status;
}

static int RegisterPassphraseProfilePost(const std::string& encryptedKey, 
                                         const std::string& salt,
                                         CredentialsManager* pCm,
                                         AtticProfileInfo& atticProfile,
                                         Entity& entity)
{
    int status = ret::A_OK;

    // Save and post
    std::string output;
    jsn::SerializeObject(&atticProfile, output);

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

        std::cout<< "CODE : " << resp.code << std::endl;
        std::cout<< "BODY : " << resp.body << std::endl;

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
                              CredentialsManager* pCm,
                              PhraseToken& pt,
                              MasterKey& out)
{
    int status = ret::A_OK;
    if(pCm) {
        // Enter passphrase to generate key.
        pCm->RegisterPassphrase(masterkey, pt); // This generates a random salt
                                                         // Sets Phrase key
        pCm->CreateMasterKeyWithPass(out, masterkey); // Create Master Key with given pass
        pCm->SetMasterKey(out);
    }
    else {
        status = -1;
    }

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

static int EnterPassphrase(const std::string& pass, std::string& salt, std::string& keyOut) {
    int status = ret::A_OK;
    Credentials cred;
    status = crypto::GenerateKeyFromPassphrase(pass, salt, cred);
    keyOut.append(reinterpret_cast<char*>(cred.m_Key), cred.GetKeySize()); 
    return status;
}


};

#endif

