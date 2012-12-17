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

    return ret::A_OK;
}

int CredentialsManager::Shutdown()
{

    return ret::A_OK;
}

int CredentialsManager::DeserializeIntoAccessToken(const std::string& buffer)
{
    if(!JsonSerializer::DeserializeObject(&m_AccessToken, buffer))
        return ret::A_FAIL_TO_DESERIALIZE_OBJECT;          
    return ret::A_OK;
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out)
{
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthToken);                       
}

void CredentialsManager::ConstructManifestPath(std::string& out)
{
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szManifest);     
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

int CredentialsManager::EnterUserNameAndPassword(const std::string& user, const std::string& pass)
{
    Credentials cred;
    m_Crypto.GenerateKeyIvFromPassphrase( user, pass, cred );
    m_MasterKey.SetCredentials(cred);

    return ret::A_OK;
}


