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
#include "atticclient.h"

namespace pass {

static int RegisterPassphraseWithAttic(const std::string& pass, 
                                       const std::string& masterkey,
                                       Client* pClient,
                                       std::string& recoverykey);

static int RegisterPassphraseProfilePost(CredentialsManager* pCm,
                                         AtticProfileInfo& atticProfile,
                                         Entity& entity);

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

void GeneratePhraseKey(const std::string& phrase, 
                       std::string& salt,
                       std::string& keyOut);

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
                                       Client* pClient,
                                       std::string& recoverykey)
{
    int status = ret::A_OK;
    // Have the passphrase, and the master key
    // generate random iv and salt
    // Inward facing method
    // Register a new passphrase.

    CredentialsManager* pCm = pClient->GetCredentialsManager();
    PhraseToken* pt = pClient->GetPhraseToken();
    Entity* pEntity = pClient->GetEntity();
    MasterKey mk;
    status = ConstructMasterKey(masterkey, 
                                pCm, 
                                *pt, 
                                mk); // also generates salt, inserts into 
                                                // phrase token
    if(status == ret::A_OK) {
        // Insert sentinel value
        mk.InsertSentinelIntoMasterKey();
        // Get Salt
        std::string salt;
        pt->GetSalt(salt);  // Phrase Token
        // Get Dirty Master Key (un-encrypted master key with sentinel values)
        std::string dirtykey;
        mk.GetMasterKeyWithSentinel(dirtykey);
        pt->SetDirtyKey(dirtykey); // Set Phrase Token
        // Enter passphrase to generate key.
        std::string phraseKey;
        //status = pCm->EnterPassphrase(pass, salt, phraseKey);
        GeneratePhraseKey(pass, salt, phraseKey);
        if(status == ret::A_OK) {
            pClient->SetPhraseKey(phraseKey);
            // Setup passphrase cred to encrypt master key
            std::string encryptedkey;
            status = EncryptKeyWithPassphrase(dirtykey, 
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
                                             *pEntity,
                                             recoverykey);
                if(status == ret::A_OK) {
                    status = RegisterPassphraseProfilePost(pCm,
                                                           atticProfile,
                                                           *pEntity);
                }
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
       
    return status; 
}

static int RegisterRecoveryQuestionsWithAttic(const std::string& masterkey,
                                              CredentialsManager* pCm,
                                              Entity& entity,
                                              std::string& q1,
                                              std::string& q2,
                                              std::string& q3,
                                              std::string& a1,
                                              std::string& a2,
                                              std::string& a3)
{
    int status = ret::A_OK;
    // Store questions on entity
    std::string compound = q1 + q2 + q3 + a1 + a2 + a3;

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
    //status = pCm->EnterPassphrase(recovery_key, salt, phraseKey);
    GeneratePhraseKey(compound, salt, phraseKey);
    if(status == ret::A_OK) {
        authcodetoken.SetPhraseKey(phraseKey);
        // Setup passphrase cred to encrypt master key
        std::string encryptedkey;
        status = EncryptKeyWithPassphrase( dirtykey, 
                                           phraseKey, 
                                           salt, 
                                           encryptedkey);
        if(status == ret::A_OK) {
            AtticProfileInfo* atticProfile = entity.GetAtticProfile();
            if(atticProfile) {
                atticProfile->SetQuestionMasterKey(encryptedkey);
                atticProfile->SetQuestionSalt(salt);
                atticProfile->SetQuestionOne(q1);
                atticProfile->SetQuestionTwo(q2);
                atticProfile->SetQuestionThree(q3);
                status = RegisterPassphraseProfilePost(pCm,
                                                       *atticProfile,
                                                       entity);
            }
            else {
                status = ret::A_FAIL_INVALID_PROFILE;
            }
        }
    }

    return status;
}

static int EnterRecoveryQuestions(CredentialsManager* pCm,
                                  Entity& entity,
                                  std::string& q1,
                                  std::string& q2,
                                  std::string& q3,
                                  std::string& a1,
                                  std::string& a2,
                                  std::string& a3,
                                  std::string& keyOut)
{
    int status = ret::A_OK;
    AtticProfileInfo* atticProfile = entity.GetAtticProfile();
    if(atticProfile) {
        std::string compound = q1 + q2 + q3 + a1 + a2 + a3;

        std::string encrypted_masterkey, salt;
        atticProfile->GetQuestionMasterKey(encrypted_masterkey);
        atticProfile->GetQuestionSalt(salt);

        std::string phraseKey;
        GeneratePhraseKey(compound, salt, phraseKey);

        if(!phraseKey.empty()) {
            std::string decrypted_masterkey;
            status = DecryptKey(encrypted_masterkey, phraseKey, salt, decrypted_masterkey);
            if(status == ret::A_OK)
                keyOut = decrypted_masterkey;
        }
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
    //status = pCm->EnterPassphrase(recovery_key, salt, phraseKey);
    GeneratePhraseKey(recovery_key, salt, phraseKey);
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

        //EnterPassphrase(unencoded_recovery_key, recovery_salt, phrasekey);
        GeneratePhraseKey(unencoded_recovery_key, recovery_salt, phrasekey);

        if(!phrasekey.empty()) {
            std::string decrypted_masterkey;
            std::cout<<"Encrypted Master Key : " << encrypted_masterkey << std::endl;
            status = DecryptKey(encrypted_masterkey, phrasekey, recovery_salt, decrypted_masterkey);
            std::cout<<"Decrypted Master Key : " << decrypted_masterkey << std::endl;
            keyOut = decrypted_masterkey;
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

void GeneratePhraseKey(const std::string& phrase, 
                       std::string& salt,
                       std::string& keyOut)
{
    Credentials cred;
    crypto::GenerateKeyFromPassphrase(phrase, salt, cred);
    // Create Passphrase token
    keyOut.append(reinterpret_cast<char*>(cred.m_Key), cred.GetKeySize()); 
}


static int RegisterPassphraseProfilePost(CredentialsManager* pCm,
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
        pCm->CreateMasterKeyWithPass(masterkey, out); // Create Master Key with given pass
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


}

#endif

