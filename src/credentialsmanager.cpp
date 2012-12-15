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
    ret::A_OK;
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out)
{
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthToken);                       
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

    int status = m_AccessToken.LoadFromFile(path);               
    std::cout<<" INSIDE THE AT STUDIO : " << m_AccessToken.GetAccessToken() << std::endl;

    AccessToken at = GetAccessTokenCopy();
    std::cout<<" INDOOR COPY : " << at.GetAccessToken() << std::endl;

    return status;
}



