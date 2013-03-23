#include "credentialsmanager.h"

#include "errorcodes.h"
#include "constants.h"
#include "utils.h"

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

    Lock();
    if(!jsn::DeserializeObject(&m_AccessToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    Unlock();

    return status;
}

int CredentialsManager::WriteOutAccessToken() {
    std::string path;
    ConstructAccessTokenPath(path);

    Lock();
    int status = m_AccessToken.SaveToFile(path);
    Unlock();
    return status;
}

int CredentialsManager::LoadAccessToken() {
    std::string path;
    ConstructAccessTokenPath(path);

    std::cout<<" ACCESSTOKEN PATH : " << path << std::endl;

    Lock();
    int status = m_AccessToken.LoadFromFile(path);
    Unlock();
    return status;
}

int CredentialsManager::DeserializeIntoPhraseToken(const std::string& buffer) {
    int status = ret::A_OK;
    Lock();
    if(!jsn::DeserializeObject(&m_PhraseToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    Unlock();

    return status;
}

int CredentialsManager::WriteOutPhraseToken() {
    std::string path;
    ConstructPhraseTokenPath(path);

    Lock();
    int status = m_PhraseToken.SaveToFile(path);
    Unlock();
    return status;
}

int CredentialsManager::LoadPhraseToken() {
    std::string path;
    ConstructPhraseTokenPath(path);

    Lock();
    int status = m_PhraseToken.LoadFromFile(path);
    Unlock();

    return status;
}

void CredentialsManager::CreateMasterKeyWithPass(const std::string& key, MasterKey& mkOut) {
    Credentials MasterKey;
    MasterKey.SetKey(key);
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
    MasterKey.GetKey(keyOut);

    return ret::A_OK;
}

//TODO:: rename this, confusing
int CredentialsManager::RegisterPassphrase(const std::string& pass, PhraseToken& ptOut) {

    int status = ret::A_OK;
    Lock();
    // TODO :: perhaps check profile if these things exist
    if(pass.empty())
        status = ret::A_FAIL_EMPTY_PASSPHRASE;

    if(status == ret::A_OK) {

        // Generate Salt
        std::string salt;
        status = crypto::GenerateSalt(salt);
        status = crypto::CheckSalt(salt);

        if(status == ret::A_OK) {
            ptOut.SetSalt(salt);

            // Generate Passphrase Key 
            Credentials cred;
            crypto::GenerateKeyFromPassphrase(pass, salt, cred);
            
            // Set the key generated from phrase
            ptOut.SetPhraseKey(reinterpret_cast<char*>(cred.m_Key));
        }
    }
    Unlock();
    return status;
}

// Depricated
int CredentialsManager::EnterPassphrase(const std::string& pass, 
                                        std::string& salt, 
                                        std::string& keyOut) 
{
    Lock();
    Credentials cred;
    crypto::GenerateKeyFromPassphrase(pass, salt, cred);
    // Create Passphrase token
    keyOut.append(reinterpret_cast<char*>(cred.m_Key), cred.GetKeySize()); 
    Unlock();

    return ret::A_OK;
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out) {
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthTokenName);                       

}

void CredentialsManager::ConstructManifestPath(std::string& out) {
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szManifestName);     
}


void CredentialsManager::ConstructPhraseTokenPath(std::string& out) {
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szPhraseTokenName);
}

