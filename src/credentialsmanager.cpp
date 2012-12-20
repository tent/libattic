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
    while(TryLock()) { sleep(0);}

    int status = ret::A_OK;
    if(!JsonSerializer::DeserializeObject(&m_AccessToken, buffer))
        status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;          

    Unlock();

    return status;
}

void CredentialsManager::ConstructAccessTokenPath(std::string& out)
{
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szAuthToken);                       

}

void CredentialsManager::ConstructManifestPath(std::string& out)
{
    // do not lock, used internally
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);      
    out.append(cnst::g_szManifest);     
}

void CredentialsManager::GetManifestPath(std::string& out)
{
    while(TryLock()) { sleep(0);}
    ConstructManifestPath(out);
    Unlock();
}


void CredentialsManager::GetAccessTokenPath(std::string& out)
{
    while(TryLock()) { sleep(0);}
    ConstructAccessTokenPath(out);
    Unlock();
}

int CredentialsManager::WriteOutAccessToken()
{
    while(TryLock()) { sleep(0);}

    std::string path;
    ConstructAccessTokenPath(path);
    int status = m_AccessToken.SaveToFile(path);  

    Unlock();
 
    return status;             
}

int CredentialsManager::LoadAccessToken()
{

    while(TryLock()) { sleep(0);}

    std::string path;
    ConstructAccessTokenPath(path);
    int status = m_AccessToken.LoadFromFile(path);

    Unlock();

    std::cout<<"-0--"<<std::endl;
    return status;               
}

int CredentialsManager::EnterUserNameAndPassword(const std::string& user, const std::string& pass)
{
    while(TryLock()) { sleep(0);}

    Credentials cred;
    m_Crypto.GenerateKeyIvFromPassphrase( user, pass, cred );
    m_MasterKey.SetCredentials(cred);

    Unlock();

    return ret::A_OK;
}

void CredentialsManager::SetConfigDirectory(const std::string& dir)
{ 
    while(TryLock()) { sleep(0);}
    m_ConfigDirectory = dir; 
    Unlock();
}

void CredentialsManager::SetAccessToken(const AccessToken& at)
{ 
    while(TryLock()) { sleep(0);}
    m_AccessToken = at; 
    Unlock();
}

void CredentialsManager::SetMasterKey(const MasterKey& mk)
{ 
    while(TryLock()) { sleep(0);}
    m_MasterKey = mk; 
    Unlock();
}

void CredentialsManager::GetMasterKeyCopy(MasterKey& key) 
{ 
    while(TryLock()) { sleep(0);}
    key = m_MasterKey; 
    Unlock();
}

void CredentialsManager::GetAccessTokenCopy(AccessToken& tk)
{ 
    while(TryLock()) { sleep(0);}
    tk = m_AccessToken; 
    Unlock();
}


