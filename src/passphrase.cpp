#include "passphrase.h"
#include "constants.h"
#include "crypto.h"
#include "logutils.h"
#include "connectionhandler.h"
#include "pagepost.h"

namespace attic { namespace pass {
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
            status = RetrieveCredentialsPost(ap);
            if(status == ret::A_OK) {
                reg = true;
                std::cout<<" OVERRIDING ... " << std::endl;
                // TODO :: when delete is implemented delete the old version
                status = DeleteCredentialsPost(ap);
            }
        }
    }
    else {
        reg = true;
    }

    if(reg) {
        std::cout<<" PREV STATUS : " << status << std::endl;
        std::cout<<" Constructing master key .... " << std::endl;
        std::cout<<" passed in passphrase : " << passphrase << std::endl;
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
        else {
            std::cout<<" COULD NOT CREATE MASTER KEY " << std::endl;
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

        //std::cout<< " MASTER KEY : " << masterkey << std::endl;
        //std::cout<< " SALT : " << salt << std::endl;
        //  generate phrase key 
        std::string phrasekey;
        GeneratePhraseKey(passphrase, salt, phrasekey);
        
        //  decrypt master key
        std::string decrypted_masterkey;
        status = DecryptKey(masterkey, phrasekey, salt, decrypted_masterkey);
        std::cout<<" DK STATUS : " << status << std::endl;
        if(status == ret::A_OK) {
            // For logging purpses
            std::string b64_mk;
            crypto::Base64EncodeString(decrypted_masterkey, b64_mk);
            std::cout<<" DECRYPTED MASTER KEY : " << b64_mk << std::endl;
            // Everything alright, phrase key is correct
            master_key_out = decrypted_masterkey;
            token_out.set_phrase_key(phrasekey);
            token_out.set_salt(salt);
        }
        else {
            std::cout<<" FAILED TO DECRYPT MASTER KEY ON ENTER PASSPHRASE " << std::endl;
        }
    }

    std::cout<<" enterpass phrase status : " << status << std::endl;
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
int Passphrase::RegisterRecoveryQuestions(const std::string& question_one,
                                          const std::string& question_two,
                                          const std::string& question_three,
                                          const std::string& answer_one,
                                          const std::string& answer_two,
                                          const std::string& answer_three) {
    int status = ret::A_OK;

    return status;
}

int Passphrase::EnterQuestionAnswerKey(const std::string& question_one,
                                       const std::string& question_two,
                                       const std::string& question_three,
                                       const std::string& answer_one,
                                       const std::string& answer_two,
                                       const std::string& answer_three) {
    int status = ret::A_OK;

    return status;
}

int Passphrase::DecryptKey(const std::string& key, 
                           const std::string& phrasekey, 
                           const std::string& salt,
                           std::string& key_out) {
    int status = ret::A_OK;

    /*
    std::cout<<" input key : " << key << std::endl;
    std::cout<<" passphrase : " << phrasekey << std::endl;
    std::cout<<" salt : " << salt << std::endl;
    */

    Credentials cred;
    cred.set_key(phrasekey);
    cred.set_iv(salt);

    std::string decrypted_key;
    //status = crypto::DecryptStringCFB(key, cred, decrypted_key);
    status = crypto::DecryptStringGCM(key, cred, decrypted_key);
    std::cout<<" DECRYPT STATUS : " << status << std::endl;
    std::cout<<" DECRYPTED KEY : " << decrypted_key << std::endl;
    if(status == ret::A_OK) {
        if(CheckSentinelBytes(decrypted_key)) {
            key_out = decrypted_key.substr(8);
        }
        else {
            status = ret::A_FAIL_SENTINEL_MISMATCH;
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
    crypto::EnterPassphrase(passphrase, salt, cred);
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
    //crypto::EncryptStringCFB(key, enc, key_out);
    crypto::EncryptStringGCM(key, enc, key_out);

}

int Passphrase::CreatePhraseToken(const std::string& master_key, PhraseToken& out) {
    int status = ret::A_OK;

    if(master_key.empty()) 
        status = ret::A_FAIL_EMPTY_PASSPHRASE;

    if(status == ret::A_OK) {
        // Generate Passphrase Key 
        Credentials cred;
        crypto::GenerateKeyFromPassphrase(master_key, cred);
        // Set the key generated from phrase
        out.set_phrase_key(cred.key());
        out.set_salt(cred.iv());
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
    }
    else { 
        url = entity_.GetPreferredServer().posts_feed();
    }

    std::string body;
    AtticPost temp = post; // TODO :: remove this copy once jsn const correctness is fixed

    std::cout<<" access token : " << access_token_.hawk_key() << std::endl;
    std::cout<<" post type :" << post.type() << std::endl;
    std::cout<<" credentials url " << url << std::endl;

    Response response;
    if(post.id().empty()) {
        jsn::SerializeObject(&temp, body);
        status = netlib::HttpPost(url, 
                                  post.type(),
                                  NULL,
                                  body,
                                  &access_token_,
                                  response);
    } 
    else {
        Parent parent;
        parent.version = temp.version().id();
        temp.PushBackParent(parent);
        jsn::SerializeObject(&temp, body);
        status = netlib::HttpPut(url, 
                                 post.type(),
                                 NULL,
                                 body,
                                 &access_token_,
                                 response);
    }

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header count : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    if(response.code == 200) {

    }
    else { 
        status = ret::A_FAIL_NON_200;
        log::LogHttpResponse("PSSSHCB8210", response);
    }

    return status;
}

int Passphrase::RetrieveCredentialsPost(AtticPost& out) { 
    int status = ret::A_OK;
    std::string url = entity_.GetPreferredServer().posts_feed();

    std::cout<<" RETRIEVING CRED POST " << std::endl;
    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_cred_type));
    std::string prm;
    params.SerializeToString(prm);
    std::cout<<" PARAMS : " << prm << std::endl;

    Response response;
    //ConnectionHandler ch;
    netlib::HttpGet(url,
               &params,
               &access_token_,
               response);

    /*
    netlib::HttpGet(url,
                   &params,
                   &access_token_,
                   response);
                   */

    std::cout<<" code : " << response.code << std::endl;
    std::cout<<" header : " << response.header.asString() << std::endl;
    std::cout<<" body : " << response.body << std::endl;

    if(response.code == 200) {
        // There should be only one, either way, take the top most in the array
        //  - later do this to sort by newest
        PagePost p;
        jsn::DeserializeObject(&p , response.body);
        Json::Value arr(Json::arrayValue);
        jsn::DeserializeJson(p.data(), arr);
        Json::ValueIterator itr = arr.begin();
        if(itr!= arr.end())
            jsn::DeserializeObject(&out, *itr);
    }
    else { 
        status = ret::A_FAIL_NON_200; 
        log::LogHttpResponse("FNDNL329Q", response);
    }

    return status;
}

int Passphrase::GetCredentialsPostCount() {
    std::string url = entity_.GetPreferredServer().posts_feed();

    std::cout<<" GET CREDENTIALS POST COUNT " << std::endl;
    UrlParams params;
    params.AddValue(std::string("types"), std::string(cnst::g_attic_cred_type));

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
    else {
        log::LogHttpResponse("41935", response);
    }

    std::cout<<" CREDENTIALS POST COUNT : " << count << std::endl;
    return count;
}

int Passphrase::DeleteCredentialsPost(AtticPost& post) { 
    int status = ret::A_OK;

    std::string postid = post.id();
    std::string url = entity_.GetPreferredServer().post();
    utils::FindAndReplace(url, "{entity}", entity_.entity(), url);
    utils::FindAndReplace(url, "{post}", post.id(), url);

    std::cout << " DELETE POST PATH : " << url << std::endl;

    Response resp;
    netlib::HttpDelete(url,
                       NULL,
                       &access_token_,
                       resp);

    if(resp.code == 200) {

    }
    else {
        log::LogHttpResponse("C3289AF", resp);
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

bool Passphrase::HasCredentialsPost() {
    if(GetCredentialsPostCount() > 0) 
        return true;
    return false;
}


}} //namespace
