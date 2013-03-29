#include "passphrase.h"
#include "constants.h"
#include "crypto.h"

namespace pass {
Passphrase::Passphrase(const Entity& entity, const AccessToken& at) {
    entity_ = entity;
    access_token_ = at;
}

Passphrase::~Passphrase() {}

int Passphrase::RegisterPassphrase(const std::string& passphrase,
                                   const std::string& masterkey,
                                   std::string& recoverykey) {
    int status = ret::A_OK;
    if(GetCredentialsPostCount() > 0 ) {
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
        // Already registered a passphrase, go enter it...
    }
    else {
        std::string encrypted_masterkey, salt;
        status = ConstructMasterKey(passphrase, masterkey, encrypted_masterkey, salt);
        if(status == ret::A_OK) {
            // Generate Recovery Key
            std::string encrypted_recovery_masterkey, recovery_salt, recovery_key;
            status = GenerateRecoveryKey(masterkey, 
                                         encrypted_recovery_masterkey,
                                         recovery_salt,
                                         recovery_key);
            recoverykey = recovery_key;

            AtticPost ap;
            ap.set_salt(salt);
            ap.set_master_key(encrypted_masterkey);
            ap.set_recovery_salt(recovery_salt);
            ap.set_recovery_master_key(encrypted_recovery_masterkey);
            // Write Out To Post
            PushAtticCredentials(ap);
        }

    }

    return status;
}

int Passphrase::EnterPassphrase(const std::string& passphrase,
                                std::string& key_out) {
    int status = ret::A_OK;
    // Get Credentials post
    AtticPost p;
    status = RetrieveCredentialsPost(p);
    if(status == ret::A_OK) {
        //  extract master key (encrypted) and salt
        std::string masterkey = p.master_key();
        std::string salt = p.salt();

        std::cout<< " MASTER KEY : " << masterkey << std::endl;
        std::cout<< " SALT : " << salt << std::endl;
        //  generate phrase key 
        std::string phrase_key;
        GeneratePhraseKey(passphrase, salt, phrase_key);
        
        //  decrypt master key
    }

    return status;
}

int Passphrase::ChangePassphrase(const std::string& old_passphrase,
                                 const std::string& new_passphrase,
                                 std::string& recoverykey) {
    int status = ret::A_OK;


    return status;
}

int Passphrase::EnterRecoveryKey(const std::string& recoverykey) {
    int status = ret::A_OK;

    return status;
}

int Passphrase::DecryptKey(const std::string& key, 
                           const std::string& phrasekey, 
                           const std::string& salt,
                           std::string& key_out) {
    int status = ret::A_OK;

    Credentials cred;



    return status;
}


int Passphrase::ConstructMasterKey(const std::string& passphrase, 
                                   const std::string& masterkey,
                                   std::string& encrypted_masterkey,
                                   std::string& salt) {
    int status = ret::A_OK;
    PhraseToken pt;
    status = CreatePhraseToken(masterkey, pt);
    if(status == ret::A_OK) {
        MasterKey mk;
        status = CreateMasterKey(masterkey, mk);
        if(status == ret::A_OK) {
            // Insert a sentinel value, for integrity check later
            mk.InsertSentinelIntoMasterKey();
            salt = pt.salt();
            std::string dirty_key = mk.key_with_sentinel();
            pt.set_dirty_key(dirty_key);
            // Generate a key based on the passphrase
            std::string phrase_key; 
            GeneratePhraseKey(passphrase, salt, phrase_key);
            // Encrypt the master key with phrase key
            EncryptKeyWithPassphrase(dirty_key, phrase_key, salt, encrypted_masterkey);
        }
    }

    return status;
}

int Passphrase::GenerateRecoveryKey(const std::string& masterkey, 
                                    std::string& encrypted_masterkey,
                                    std::string& salt,
                                    std::string& recovery_key) {
    int status = ret::A_OK;
    std::string recovery_phrase;
    utils::GenerateRandomString(recovery_phrase, 32);
    std::string encrypted_master_key;
    status = ConstructMasterKey(recovery_phrase, masterkey, encrypted_masterkey, salt);
    if(status == ret::A_OK) {
        crypto::Base32EncodeString(recovery_phrase, recovery_key);
    }
    return status;
}

void Passphrase::GeneratePhraseKey(const std::string& passphrase, 
                                   const std::string& salt,
                                   std::string& key_out) {
    Credentials cred;
    crypto::GenerateKeyFromPassphrase(passphrase, salt, cred);
    // Create Passphrase token
    key_out.append(reinterpret_cast<char*>(cred.m_Key), cred.GetKeySize()); 
}

void Passphrase::EncryptKeyWithPassphrase(const std::string& key, 
                                          const std::string& phrasekey, 
                                          const std::string& salt,
                                          std::string& key_out) {
    // encryption credentials
    Credentials enc;
    enc.SetKey(phrasekey);
    enc.SetIv(salt);
    // Encrypt MasterKey with passphrase key
    crypto::EncryptStringCFB(key, enc, key_out);
}

int Passphrase::CreatePhraseToken(const std::string& masterkey, PhraseToken& out) {
    int status = ret::A_OK;

    if(masterkey.empty()) 
        status = ret::A_FAIL_EMPTY_PASSPHRASE;

    if(status == ret::A_OK) {
        // Generate Salt
        std::string salt;
        status = crypto::GenerateSalt(salt);
        status = crypto::CheckSalt(salt);

        if(status == ret::A_OK) {
            out.set_salt(salt);

            // Generate Passphrase Key 
            Credentials cred;
            crypto::GenerateKeyFromPassphrase(masterkey, salt, cred);
            
            // Set the key generated from phrase
            out.set_phrase_key(reinterpret_cast<char*>(cred.m_Key));
        }
    }

    return status;
}

int Passphrase::CreateMasterKey(const std::string& masterkey, MasterKey& out) {
    int status = ret::A_OK;
    Credentials mk;
    mk.SetKey(masterkey);

    out.SetCredentials(mk);
    return status;
}

int Passphrase::PushAtticCredentials(const AtticPost& post) {
    int status = ret::A_OK;
    std::string url = entity_.GetPreferredServer().posts_feed();

    std::string body;
    AtticPost temp = post; // TODO :: remove this copy once jsn const correctness is fixed
    jsn::SerializeObject(&temp, body);

    std::cout<<" access token : " << access_token_.GetMacKey() << std::endl;
    std::cout<<" post type :" << post.type() << std::endl;

    Response response;
    status = netlib::HttpPost(url, 
                              post.type(),
                              NULL,
                              body,
                              &access_token_,
                              response);

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header count : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    if(response.code == 200) {

    }
    else { 
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int Passphrase::RetrieveCredentialsPost(AtticPost& out) { 
    int status = ret::A_OK;
    std::string url = entity_.GetPreferredServer().posts_feed();

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_attic_cred_type));

    Response response;
    netlib::HttpGet(url,
                   &params,
                   &access_token_,
                   response);

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    if(response.code == 200) {
        // There should be only one, either way, take the top most in the array
        //  - later do this to sort by newest
        Json::Value arr(Json::arrayValue);
        jsn::DeserializeJson(response.body, arr);
        Json::ValueIterator itr = arr.begin();
        if(itr!= arr.end())
            jsn::DeserializeObject(&out, *itr);
    }
    else { 
        status = ret::A_FAIL_NON_200; 
    }

    return status;
}

int Passphrase::GetCredentialsPostCount() {
    std::string url = entity_.GetPreferredServer().posts_feed();

    UrlParams params;
    params.AddValue(std::string("post_types"), std::string(cnst::g_attic_cred_type));

    Response response;
    netlib::HttpHead(url,
                    &params,
                    &access_token_,
                    response);

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    int count = -1;
    if(response.code == 200) {
        if(response.header.HasValue("Count"))
            count = atoi(response.header["Count"].c_str());
    }
    return count;
}


} //namespace
