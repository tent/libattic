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
                                   std::string& recoverykey,
                                   bool override) {
    int status = ret::A_OK;
    bool reg = false;
    AtticPost ap;
    if(GetCredentialsPostCount() > 0 ) {
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
        if(override) {
            // Retrieve Attic Post
            if(RetrieveCredentialsPost(ap) == ret::A_OK)
                reg = true;
        }
    }
    else {
        reg = true;
    }

    if(reg) {
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
                                PhraseToken& token_out,
                                std::string& master_key_out) {
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
        std::string phrasekey;
        GeneratePhraseKey(passphrase, salt, phrasekey);
        
        //  decrypt master key
        std::string decrypted_masterkey;
        status = DecryptKey(masterkey, phrasekey, salt, decrypted_masterkey);
        if(status == ret::A_OK) {
            // Everything alright, phrase key is correct
            master_key_out = decrypted_masterkey;
            std::cout<<" DECRYPTED MASTER KEY : " << master_key_out << std::endl;
            std::cout<<" PHRASE KEY : " << phrasekey << std::endl;
            token_out.set_phrase_key(phrasekey);
            token_out.set_salt(salt);
        }
    }

    return status;
}

int Passphrase::ChangePassphrase(const std::string& old_passphrase,
                                 const std::string& new_passphrase,
                                 std::string& recoverykey) {
    int status = ret::A_OK;

    PhraseToken pt;
    std::string master_key;
    // Enter old passphrase
    status = EnterPassphrase(old_passphrase, pt, master_key);
    if(status == ret::A_OK) {
        //  get master key 
        //  register new passphrase with master key
        status = RegisterPassphrase(new_passphrase, master_key, recoverykey, true);
    }

    return status;
}

int Passphrase::EnterRecoveryKey(const std::string& recoverykey, std::string& temp_out) {
    int status = ret::A_OK;

    AtticPost ap;
    status = RetrieveCredentialsPost(ap);
    if(status == ret::A_OK) { 
        std::string recovery_masterkey = ap.recovery_master_key();
        std::string recovery_salt = ap.recovery_salt();

        std::string recovery_key;
        crypto::Base32DecodeString(recoverykey, recovery_key);

        std::string phrasekey;
        GeneratePhraseKey(recovery_key, recovery_salt, phrasekey);

        std::string decrypted_masterkey;
        status = DecryptKey(recovery_masterkey, phrasekey, recovery_salt, decrypted_masterkey);

        if(status == ret::A_OK) {
            std::string temp_pass;                        
            utils::GenerateRandomString(temp_pass, 16);   
            std::string new_recovery_key;                
            status = RegisterPassphrase(temp_pass, decrypted_masterkey, new_recovery_key, true);
            if(status == ret::A_OK)
                temp_out = temp_pass;
        }
    }

    return status;
}

int Passphrase::DecryptKey(const std::string& key, 
                           const std::string& phrasekey, 
                           const std::string& salt,
                           std::string& key_out) {
    int status = ret::A_OK;

    Credentials cred;
    cred.set_key(phrasekey);
    cred.set_iv(salt);

    std::string decrypted_key;
    status = crypto::DecryptStringCFB(key, cred, decrypted_key);
    if(status == ret::A_OK) {
        if(CheckSentinelBytes(decrypted_key)) {
            key_out = decrypted_key.substr(8);
        }
    }
    else {
        status = ret::A_FAIL_SENTINEL_MISMATCH;
    }

    return status;
}

bool Passphrase::CheckSentinelBytes(const std::string& in) {
    if(in.size() >= 8) {
        if(in.substr(0,4) == in.substr(4,4))
            return true;
    }
    return false;
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
    key_out = cred.key();
}

void Passphrase::EncryptKeyWithPassphrase(const std::string& key, 
                                          const std::string& phrasekey, 
                                          const std::string& salt,
                                          std::string& key_out) {
    // encryption credentials
    Credentials enc;
    enc.set_key(phrasekey);
    enc.set_iv(salt);
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
            out.set_phrase_key(cred.key());
        }
    }

    return status;
}

int Passphrase::CreateMasterKey(const std::string& masterkey, MasterKey& out) {
    int status = ret::A_OK;
    Credentials mk;
    mk.set_key(masterkey);
    out.SetCredentials(mk);
    return status;
}

int Passphrase::PushAtticCredentials(const AtticPost& post) {
    int status = ret::A_OK;
    std::string url;
    if(!post.id().empty()) {
        url = entity_.GetPreferredServer().post();
        utils::FindAndReplace(url, "{entity}", entity_.entity(), url);
        utils::FindAndReplace(url, "{post}", post.id(), url);
        std::cout<<" FIND AND REPLACE URL : " << url << std::endl;
    }
    else { 
        url = entity_.GetPreferredServer().posts_feed();
    }

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
