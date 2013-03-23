#include "atticclient.h"

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
    if(!status)
        status = InitializeCredentialsManager();
    return status;
}

int Client::Shutdown() { 
    int status = ret::A_OK;


    return status;
}

int Client::InitializeFileManager() {
    int status = ret::A_OK;
    std::string config, working, temp;
    status = fs::GetCanonicalPath(m_ConfigDirectory, config);
    status = fs::GetCanonicalPath(m_WorkingDirectory, working);
    status = fs::GetCanonicalPath(m_TempDirectory, temp);

    if(status == ret::A_OK) {
        status = m_FileManager.StartupFileManager(config, working, temp);
    }
    return status;
}

int Client::InitializeCredentialsManager() {
    int status = ret::A_OK;
    std::string config;
    status = fs::GetCanonicalPath(m_ConfigDirectory, config);

    if(status == ret::A_OK) {
        m_CredentialsManager.SetConfigDirectory(config);
        status = m_CredentialsManager.Initialize();
    }

    return status;
}

int Client::ShutdownFileManager() {
    int status = ret::A_OK;
    status = m_FileManager.ShutdownFileManager();
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

int Client::LoadAppFromFile() { 
    int status = ret::A_OK;
    std::string savepath;
    ConstructAppPath(savepath);
    status = m_App.LoadFromFile(savepath);

    return status;
}

int Client::LoadEntity(bool override) {
    int status = ret::A_OK;

    std::string entpath;
    ConstructEntityFilepath(entpath);
    
    // Attempt to load from file
    status = m_Entity.LoadFromFile(entpath);
    // Check for master key
    if(!m_Entity.HasAtticProfileMasterKey())
        status = ret::A_FAIL_INVALID_MASTERKEY;

    // If not or overrride, lets attempt discovery
    if(status != ret::A_OK || override) {
        if(override) m_Entity.Reset();
        status = m_Entity.Discover(m_EntityUrl, &m_At);

        if(status == ret::A_OK)
            m_Entity.WriteToFile(entpath);
    }
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

