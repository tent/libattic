#include "atticclient.h"

#include <iostream>

#include "utils.h"
#include "constants.h"
#include "filesystem.h"

Client::Client(const std::string& workingdir, 
               const std::string& configdir, 
               const std::string& tempdir, 
               const std::string& entityurl) 
{
    m_WorkingDirectory = workingdir;
    m_ConfigDirectory = configdir;
    m_TempDirectory = tempdir;
    m_EntityUrl = entityurl;
}

Client::~Client() {}

int Client::Initialize() {
    int status = ret::A_OK;
    if(!status)
        status = InitializeFileManager();
    std::cout << " file manager init status : " << status << std::endl;
    if(!status)
        status = InitializeCredentialsManager();
    return status;
}

int Client::Shutdown() { 
    int status = ret::A_OK;

    std::cout<<"shutting down file manager ... " << std::endl;
    status = ShutdownFileManager();
    std::cout<<"shutting down cred manager ... " << std::endl;
    status = ShutdownCredentialsManager();

    return status;
}

int Client::InitializeFileManager() {
    int status = ret::A_OK;
    std::string config, working, temp;
    status = fs::GetCanonicalPath(m_ConfigDirectory, config);
    status = fs::GetCanonicalPath(m_WorkingDirectory, working);
    status = fs::GetCanonicalPath(m_TempDirectory, temp);

    if(status == ret::A_OK) {
        std::cout<<" Initializing file manager ... " << std::endl;
        status = m_FileManager.Initialize(config, working, temp);
    }
    return status;
}

int Client::InitializeCredentialsManager() {
    int status = ret::A_OK;
    std::string config;
    std::cout<<" config directory : " << m_ConfigDirectory << std::endl;
    status = fs::GetCanonicalPath(m_ConfigDirectory, config);

    if(status == ret::A_OK) {
        m_CredentialsManager.SetConfigDirectory(config);
        status = m_CredentialsManager.Initialize();
    }

    return status;
}

int Client::ShutdownFileManager() {
    int status = ret::A_OK;
    status = m_FileManager.Shutdown();
    return status;
}

int Client::ShutdownCredentialsManager() {
    int status = ret::A_OK;
    m_CredentialsManager.Shutdown();
    return status;
}

int Client::LoadAccessToken() { 
    int status = ret::A_OK;
    status = m_CredentialsManager.LoadAccessToken();
    if(status == ret::A_OK)
        m_CredentialsManager.GetAccessTokenCopy(m_At);

    std::string at = m_At.GetAccessToken();
    std::cout<<" STATUS : " << status << std::endl;
    std::cout<<" ACCESS TOKEN : " << at << std::endl;
    return status;
}

int Client::LoadPhraseToken() { 
    int status = ret::A_OK;

    std::string path;
    ConstructPhraseTokenFilepath(path);

    status = m_PhraseToken.LoadFromFile(path);
    if(status != ret::A_OK) {
        // Assume no file
        // Extract info from entity
        status = ExtractPhraseToken(m_PhraseToken);
        if(status == ret::A_OK) {
            m_PhraseToken.SaveToFile(path);
        }

    }

    return status;
}

void Client::SetPhraseKey(const std::string& key) {
    std::string path;
    ConstructPhraseTokenFilepath(path);
    m_PhraseToken.SetPhraseKey(key);
    m_PhraseToken.SaveToFile(path);
}

int Client::ExtractPhraseToken(PhraseToken& out) {
   // TODO :: this with app post 

    //return status;
    return -1;
}

int Client::LoadAppFromFile() { 
    int status = ret::A_OK;
    std::string savepath;
    ConstructAppPath(savepath);
    status = m_App.LoadFromFile(savepath);

    return status;
}

int Client::LoadEntity(bool override) {
    int status = ret::A_OK;

    //VO3
    /*
    std::string entpath;
    ConstructEntityFilepath(entpath);
    
    // Attempt to load from file
    status = m_Entity.LoadFromFile(entpath);

    // If not or overrride, lets attempt discovery
    if(status != ret::A_OK || override) {
        status = m_Entity.Discover(m_EntityUrl, &m_At);

        if(status == ret::A_OK)
            m_Entity.WriteToFile(entpath);
    }
    */
    return status;
}

int Client::SaveAppToFile() { 
    int status = ret::A_OK;
    std::string savepath;
    ConstructAppPath(savepath);
    status = m_App.SaveToFile(savepath);

    return status;
}
/*
void Client::StartupAppInstance(const std::string& appName,
                                const std::string& appDescription,
                                const std::string& url,
                                const std::string& icon,
                                std::vector<std::string>& uris,
                                std::vector<std::string>& scopes)
{
    m_App.SetAppName(appName);
    m_App.SetAppDescription(appDescription);
    m_App.SetAppURL(url);
    m_App.SetAppIcon(icon);

    m_App.SetRedirectUris(uris);
    m_App.SetScopes(scopes);
}
*/

void Client::ConstructAppPath(std::string& out) {
    // Construct path
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);
    out.append(cnst::g_szAppDataName);
}

void Client::ConstructEntityFilepath(std::string& out) {
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);
    out.append(cnst::g_szEntityName);
}

void Client::ConstructPhraseTokenFilepath(std::string& out) {
    out = m_ConfigDirectory;
    utils::CheckUrlAndAppendTrailingSlash(out);
    out += cnst::g_szPhraseTokenName;
}
