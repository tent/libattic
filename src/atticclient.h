#ifndef ATTICCLIENT_H_
#define ATTICCLIENT_H_

#include <string>

#include "tentapp.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "entity.h"
#include "accesstoken.h"
#include "phrasetoken.h"

class Client {
    void ConstructAppPath(std::string& out);
    void ConstructEntityFilepath(std::string& out);
    void ConstructPhraseTokenFilepath(std::string& out);

    int InitializeFileManager();
    int InitializeCredentialsManager();

    int ShutdownFileManager();
    int ShutdownCredentialsManager();

    int ExtractPhraseToken(PhraseToken& out);
public:
    Client(const std::string& workingdir, 
           const std::string& configdir, 
           const std::string& tempdir, 
           const std::string& entityurl);

    ~Client();

    int Initialize();
    int Shutdown();

    int LoadAccessToken();
    int LoadPhraseToken();
    int LoadAppFromFile();
    int LoadEntity(bool override=false);

    int SaveAppToFile();
    int SavePhraseToken();

    void SetTentApp(const TentApp& app) { m_App = app; }

    void SetPhraseKey(const std::string& key);
    
    TentApp* GetTentApp()                       { return &m_App; }
    FileManager* GetFileManager()               { return &m_FileManager; }
    CredentialsManager* GetCredentialsManager() { return &m_CredentialsManager; }
    Entity* GetEntity()                         { return &m_Entity; }
    PhraseToken* GetPhraseToken()               { return &m_PhraseToken; }

    // Return copies
    AccessToken GetAccessTokenCopy()                { return m_At; }
    PhraseToken GetPhraseTokenCopy()                { return m_PhraseToken; }

    void SetWorkingDirectory(const std::string& dir) { m_WorkingDirectory = dir; }
    void SetConfigDirectory(const std::string& dir) { m_ConfigDirectory = dir; }
    void SetTempDirectory(const std::string& dir) { m_TempDirectory = dir; }
    void SetEntityUrl(const std::string& url) { m_EntityUrl = url; }

    std::string GetWorkingDirectory() const { return m_WorkingDirectory; }
    std::string GetConfigDirectory() const  { return m_ConfigDirectory; }
    std::string GetTempDirectory() const    { return m_TempDirectory; } 
    std::string GetEntityUrl() const        { return m_EntityUrl; }

private:
    TentApp             m_App;
    FileManager         m_FileManager;
    CredentialsManager  m_CredentialsManager;
    Entity              m_Entity;
    AccessToken         m_At;
    PhraseToken         m_PhraseToken;

    std::string m_WorkingDirectory;
    std::string m_ConfigDirectory;
    std::string m_TempDirectory;
    std::string m_EntityUrl;
};

#endif

