#include "credentialsmanager.h"

#include "errorcodes.h"
#include "constants.h"
#include "utils.h"

// TODO:: come back and audit this, lots of useless locking going on. after 0.1 come and clean that up
CredentialsManager::CredentialsManager() {}
CredentialsManager::~CredentialsManager() {}

int CredentialsManager::Initialize() {
    // Pull Profile Info to get latest data
    // Search for and load/create access token
    //
    // Search for and load/create phrase token
    return ret::A_OK;
}

int CredentialsManager::Shutdown(){
    return ret::A_OK;
}

int CredentialsManager::DeserializeIntoAccessToken(const std::string& buffer) {
    Lock();
    int status = ret::A_OK;
    if(!jsn::DeserializeObject(&m_AccessToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    Unlock();

    return status;
}

int CredentialsManager::WriteOutAccessToken() {
    Lock();
    std::string path;
    ConstructAccessTokenPath(path);
    Unlock();

    return m_AccessToken.SaveToFile(path);  
}

int CredentialsManager::LoadAccessToken() {
    Lock();
    std::string path;
    ConstructAccessTokenPath(path);
    Unlock();

    return m_AccessToken.LoadFromFile(path);
}

int CredentialsManager::DeserializeIntoPhraseToken(const std::string& buffer) {
    Lock();
    int status = ret::A_OK;
    if(!jsn::DeserializeObject(&m_PhraseToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    Unlock();

    return status;
}

int CredentialsManager::WriteOutPhraseToken() {
    Lock();
    std::string path;
    ConstructPhraseTokenPath(path);
    Unlock();

    return m_PhraseToken.SaveToFile(path);
}

int CredentialsManager::LoadPhraseToken() {
    Lock();
    std::string path;
    ConstructPhraseTokenPath(path);
    Unlock();

    return m_PhraseToken.LoadFromFile(path);
}

int CredentialsManager::CreateMasterKeyWithPass( MasterKey& mkOut, const std::string& key) {
    Lock();
    Credentials MasterKey;
    MasterKey.SetKey(key);
    mkOut.SetCredentials(MasterKey);
    Unlock();

    return ret::A_OK;
}

int CredentialsManager::GenerateMasterKey(MasterKey& mkOut) {
    Lock();
    // Create Master Key
    Credentials MasterKey;
    crypto::GenerateCredentials(MasterKey);

    mkOut.SetCredentials(MasterKey);
    Unlock();

    return ret::A_OK;
}

int CredentialsManager::GenerateMasterKey(std::string& keyOut) {
    Lock();
    // Create Master Key
    Credentials MasterKey;
    crypto::GenerateCredentials(MasterKey);

    MasterKey.GetKey(keyOut);
    Unlock();

    return ret::A_OK;
}

//TODO:: rename this, confusing
int CredentialsManager::RegisterPassphrase(const std::string& pass, PhraseToken& ptOut) {
    Lock();
    // TODO :: perhaps check profile if these things exist
    if(pass.empty())
        return ret::A_FAIL_EMPTY_PASSPHRASE;

    int status = ret::A_OK;
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

