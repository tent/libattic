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
#include "atticpost.h"
#include "privatekeypost.h"

namespace attic { namespace pass {

class Passphrase {
    int PushAtticCredentials(const AtticPost& post);

    int ConstructMasterKey(const std::string& passphrase, 
                           const std::string& masterkey,
                           std::string& encrypted_masterkey,
                           std::string& salt);

    int CreatePhraseToken(const std::string& master_key, PhraseToken& out);
    int CreateMasterKey(const std::string& master_key, MasterKey& out);

    void GeneratePhraseKey(const std::string& passphrase, 
                           const std::string& salt,
                           std::string& key_out);

    int GenerateRecoveryKey(const std::string& masterkey, 
                            std::string& encrypted_masterkey,
                            std::string& salt,
                            std::string& recovery_key);

    void EncryptKeyWithPassphrase(const std::string& key, 
                                  const std::string& phrasekey, 
                                  const std::string& salt,
                                  std::string& key_out);

    int DecryptKey(const std::string& key, 
                   const std::string& phrasekey, 
                   const std::string& salt,
                   std::string& key_out);

    int RetrieveCredentialsPost(AtticPost& out);
    int RetrievePrivateKeyPost(PrivateKeyPost& out);

    int GetCredentialsPostCount();
    int GetPublicKeyPostCount();
    int GetPrivateKeyPostCount();


    bool CheckSentinelBytes(const std::string& in);

    int DeleteCredentialsPost(AtticPost& post);

    int RegisterPublicKey(const std::string& public_key);
    int RegisterPrivateKey(const std::string& public_key, const std::string& private_key);
public:
    Passphrase(const Entity& entity, const AccessToken& at);
    ~Passphrase();

    int RegisterPassphrase(const std::string& passphrase,
                           const std::string& masterkey,
                           std::string& recoverykey,
                           bool override = false);

    int RegisterPublicPrivateKeyPair(const std::string& public_key, 
                                     const std::string& private_key);

    int RetrievePublicPrivateKeyPair(std::string& public_out,
                                     std::string& private_out);

    int EnterPassphrase(const std::string& passphrase,
                        PhraseToken& token_out,
                        std::string& master_key_out);

    int ChangePassphrase(const std::string& old_passphrase,
                         const std::string& new_passphrase,
                         std::string& recoverykey);

    int EnterRecoveryKey(const std::string& recoverykey,
                         std::string& temp_out);

    int RegisterRecoveryQuestions(const std::string& question_one,
                                  const std::string& question_two,
                                  const std::string& question_three,
                                  const std::string& answer_one,
                                  const std::string& answer_two,
                                  const std::string& answer_three);

    int EnterQuestionAnswerKey(const std::string& question_one,
                               const std::string& question_two,
                               const std::string& question_three,
                               const std::string& answer_one,
                               const std::string& answer_two,
                               const std::string& answer_three);

    bool HasCredentialsPost();
private:
    AccessToken access_token_;
    Entity entity_;
};

}} //namespace
#endif

