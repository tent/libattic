#include "credentialsmanager.h"

#include "errorcodes.h"
#include "constants.h"
#include "utils.h"

namespace attic { 

// TODO:: come back and audit this, lots of useless locking going on. after 0.1 come and clean that up
CredentialsManager::CredentialsManager() {}
CredentialsManager::~CredentialsManager() {}

int CredentialsManager::Initialize() {
    // Search for and load/create phrase token
    return ret::A_OK;
}

int CredentialsManager::Shutdown(){
    return ret::A_OK;
}

int CredentialsManager::DeserializeIntoAccessToken(const std::string& buffer) {
    int status = ret::A_OK;
    at_mtx_.Lock();
    if(!jsn::DeserializeObject(&access_token_, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    at_mtx_.Unlock();
    return status;
}

int CredentialsManager::WriteOutAccessToken() {
    std::string path;
    ConstructAccessTokenPath(path);
    at_mtx_.Lock();
    int status = access_token_.SaveToFile(path);
    at_mtx_.Unlock();
    return status;
}

int CredentialsManager::LoadAccessToken() {
    std::string path;
    ConstructAccessTokenPath(path);

    std::cout<<" ACCESSTOKEN PATH : " << path << std::endl;

    at_mtx_.Lock();
    int status = access_token_.LoadFromFile(path);
    at_mtx_.Unlock();
    return status;
}

int CredentialsManager::DeserializeIntoPhraseToken(const std::string& buffer) {
    int status = ret::A_OK;
    pt_mtx_.Lock();
    if(!jsn::DeserializeObject(&phrase_token_, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    pt_mtx_.Unlock();

    return status;
}

int CredentialsManager::WriteOutPhraseToken() {
    std::string path;
    ConstructPhraseTokenPath(path);

    pt_mtx_.Lock();
    int status = phrase_token_.SaveToFile(path);
    pt_mtx_.Unlock();
    return status;
}

int CredentialsManager::LoadPhraseToken() {
    std::string path;
    ConstructPhraseTokenPath(path);

    pt_mtx_.Lock();
    int status = phrase_token_.LoadFromFile(path);
    pt_mtx_.Unlock();

    return status;
}

void CredentialsManager::CreateMasterKeyWithPass(const std::string& key, MasterKey& mkOut) {
    Credentials MasterKey;
    MasterKey.set_key(key);
    mkOut.SetCredentials(MasterKey);
}

void CredentialsManager::GenerateMasterKey(MasterKey& mkOut) {
    // Create Master Key
    Credentials MasterKey;
    crypto::GenerateCredentials(MasterKey);
    mkOut.SetCredentials(MasterKey);
}

int CredentialsManager::GenerateMasterKey(std::string& keyOut) {
    // Create Master Key
    Credentials MasterKey;
    crypto::GenerateCredentials(MasterKey);
    keyOut = MasterKey.key();
    return ret::A_OK;
}

void CredentialsManager::GeneratePublicPrivateKeyPair(std::string& public_out,
        std::string& private_out) {
    std::string public_key, private_key;
    crypto::GeneratePublicAndPrivateKey(public_key, private_key);

    pk_mtx_.Lock();
    sk_mtx_.Lock();
    crypto::Base64EncodeString(public_key, public_key_);
    crypto::Base64EncodeString(private_key, private_key_);
    public_out.append(public_key_.c_str(), public_key_.size());
    private_out.append(private_key_.c_str(), private_key_.size());
    sk_mtx_.Unlock();
    pk_mtx_.Unlock();
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out) {
    // do not lock, used internally
    // Construct path
    std::cout<<" cred manager config dir : " << config_directory_ << std::endl;
    out = config_directory_;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthTokenName);                       
}

void CredentialsManager::ConstructManifestPath(std::string& out) {
    // do not lock, used internally
    // Construct path
    out = config_directory_;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szManifestName);     
}


void CredentialsManager::ConstructPhraseTokenPath(std::string& out) {
    out = config_directory_;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szPhraseTokenName);
}

} //namespace
