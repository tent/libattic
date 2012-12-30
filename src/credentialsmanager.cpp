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

int CredentialsManager::EnterPassphrase(const std::string& pass, std::string& salt)
{
    Credentials cred;
    m_Crypto.GenerateKeyFromPassphrase(pass, salt, cred);
    // Create Passphrase token
    std::string key;
    key.append(reinterpret_cast<char*>(cred.key), cred.GetKeySize()); 
    m_PhraseToken.SetKey(key);
    // Write it out
    WriteOutPhraseToken();

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

