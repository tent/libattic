#include "credentialsmanager.h"

#include "errorcodes.h"
#include "constants.h"
#include "utils.h"

CredentialsManager::CredentialsManager()
{

}

CredentialsManager::~CredentialsManager()
{

}

int CredentialsManager::Initialize()
{
    // Pull Profile Info to get latest data
    // Search for and load/create access token
    //
    // Search for and load/create phrase token

    return ret::A_OK;
}

int CredentialsManager::Shutdown()
{

    return ret::A_OK;
}

int CredentialsManager::DeserializeIntoAccessToken(const std::string& buffer)
{
    int status = ret::A_OK;
    if(!JsonSerializer::DeserializeObject(&m_AccessToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          

    return status;
}

int CredentialsManager::WriteOutAccessToken()
{
    std::string path;
    ConstructAccessTokenPath(path);
    return m_AccessToken.SaveToFile(path);  
}

int CredentialsManager::LoadAccessToken()
{
    std::string path;
    ConstructAccessTokenPath(path);
    return m_AccessToken.LoadFromFile(path);
}

int CredentialsManager::DeserializeIntoPhraseToken(const std::string& buffer)
{
    int status = ret::A_OK;
    if(!JsonSerializer::DeserializeObject(&m_PhraseToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          

    return status;
}

int CredentialsManager::WriteOutPhraseToken()
{
    std::string path;
    ConstructPhraseTokenPath(path);
    return m_PhraseToken.SaveToFile(path);
}

int CredentialsManager::LoadPhraseToken()
{
    std::string path;
    ConstructPhraseTokenPath(path);
    return m_PhraseToken.LoadFromFile(path);
}

/*
int CredentialsManager::EnterUserNameAndPassword(const std::string& user, const std::string& pass)
{
    Credentials cred;
    m_Crypto.GenerateKeyIvFromPassphrase( user, pass, cred );
    m_MasterKey.SetCredentials(cred);

    return ret::A_OK;
}
*/

int CredentialsManager::GenerateMasterKey( MasterKey& mkOut)
{
    // Create Master Key
    Credentials MasterKey;
    m_Crypto.GenerateCredentials(MasterKey);

    mkOut.SetCredentials(MasterKey);

    return ret::A_OK;
}

int CredentialsManager::RegisterPassphrase( const std::string& pass, 
                                            PhraseToken& ptOut)
{
    // TODO :: perhaps check profile if these things exist
    if(pass.empty())
        return ret::A_FAIL_EMPTY_PASSPHRASE;

    // Generate Salt
    std::string salt;
    m_Crypto.CheckSalt(salt);

    ptOut.SetSalt(salt);

    // Generate Passphrase Key 
    Credentials cred;
    m_Crypto.GenerateKeyFromPassphrase(pass, salt, cred);
    
    // Set the key generated from phrase
    ptOut.SetPhraseKey(reinterpret_cast<char*>(cred.m_Key));

    return ret::A_OK;
}

int CredentialsManager::EnterPassphrase( const std::string& pass, 
                                         std::string& salt, 
                                         std::string& keyOut)
{
    Credentials cred;
    m_Crypto.GenerateKeyFromPassphrase(pass, salt, cred);
    // Create Passphrase token
    keyOut.append(reinterpret_cast<char*>(cred.m_Key), cred.GetKeySize()); 

    return ret::A_OK;
}

int CredentialsManager::GenerateMasterKey()
{
    // Generate random master key that will be used to encrypt all files,
    // This key will be encrypted itself with the passphrase - scrypt

    return ret::A_OK;
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out)
{
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthTokenName);                       

}

void CredentialsManager::ConstructManifestPath(std::string& out)
{
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szManifestName);     
}


void CredentialsManager::ConstructPhraseTokenPath(std::string& out)
{
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szPhraseTokenName);
}

