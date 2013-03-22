#include "libattic.h"
#include "callbackhandler.h"

#include <fstream>
#include <string>
#include <sstream>

#include "credentialsmanager.h"
#include "filemanager.h"
#include "entity.h"

#include "utils.h"
#include "errorcodes.h"
#include "apputils.h"
#include "tentapp.h"
#include "jsonserializable.h"
#include "urlparams.h"
#include "post.h"

#include "taskfactory.h"
#include "taskarbiter.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "synctask.h"

#include "constants.h"
#include "credentials.h"
#include "profile.h"

#include "libatticutils.h"
#include "log.h"

#include "taskmanager.h"
#include "filesystem.h"

#include "event.h"
#include "callbackhandler.h"
#include "passphrase.h"
#include "tentclient.h"

#include <cbase64.h>
#include "atticclient.h"
// TODO :: 
// Things to wrap with mutexes
//  - app
//  - filemanager
//  - connectionmanager
//  - access token? (perhaps provide a copy per operation)

// TODO :: introduce queue'ing mechanism, to protect against multiple
//         file operation spam
//         probably map string enum::state
//         on each operation lock file set state
//         return new error codes if file is in the process of being processed.
//
// TODO :: Consider moving TentApp into Credentials Manager

/*
static TentApp*             g_pApp = NULL;              // move to client
static FileManager*         g_pFileManager = NULL;      // move to client
static CredentialsManager*  g_pCredManager = NULL;      // move to client
*/

static TaskManager*         g_pTaskManager = NULL;      // move to service

//static AccessToken          g_AccessToken;              // move to client
static CallbackHandler      g_CallbackHandler;          // move to service
//static TaskArbiter g_Arb;

// Directories
static std::string g_WorkingDirectory;      // move to client
static std::string g_ConfigDirectory;// move to client
static std::string g_TempDirectory;// move to client

// var
static std::string g_EntityUrl;// move to client
static std::string g_AuthorizationURL;// move to client
static PhraseToken  g_Pt;// move to client
static Entity g_Entity;// move to client

static bool g_bEnteredPassphrase = false;
static bool g_bLibInitialized = false;

// Local utility functions
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi); // Depricated
static int PutFile(const char* szUrl, const char* szFilePath, FileInfo* fi); // Depricated

// inward faceing functions
int SetWorkingDirectory(const char* szDir);
int SetConfigDirectory(const char* szDir);
int SetTempDirectory(const char* szDir);

int LoadEntity(bool override = false);
int SaveEntity();

int LoadPhraseToken();
int SavePhraseToken(PhraseToken& pt);
int LoadMasterKey(); // call with a valid phrase token

void GetPhraseTokenFilepath(std::string& out);
void GetEntityFilepath(std::string& out);

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv);

int IsLibInitialized(bool checkPassphrase = true);

Client* pClient;

//TODO TESTING METHODS REMOVE
//FileManager* GetFileManager() { return g_pFileManager; }

//////// API start
int InitLibAttic( const char* szWorkingDirectory, 
                  const char* szConfigDirectory,
                  const char* szTempDirectory,
                  const char* szLogDirectory,
                  const char* szEntityURL,
                  unsigned int threadCount)
{

    int status = ret::A_OK;
    // Init sequence ORDER MATTERS
    utils::SeedRand();
    SetConfigDirectory(szConfigDirectory);
    SetWorkingDirectory(szWorkingDirectory);
    SetTempDirectory(szTempDirectory);

    std::string t;
    pClient = new Client(szWorkingDirectory,
                         szConfigDirectory,
                         szTempDirectory,
                         szEntityURL);
                          
                         
                         
    pClient->LoadAppFromFile();
    pClient->LoadAccessToken();

    // Initialize logging
    alog::InitializeLogging(szLogDirectory);
    alog::Log(Logger::DEBUG, "Init");

    //status = LoadAppFromFile();
    if(status == ret::A_OK)
    {
        // Essential
        status = SetEntityUrl(szEntityURL);

        if(status != ret::A_OK) 
            alog::Log(Logger::ERROR, "Failed to set entity url");

        /*
        status = liba::InitializeFileManager( &g_pFileManager,
                                              g_ConfigDirectory,
                                              g_WorkingDirectory,
                                              g_TempDirectory);

        status = liba::InitializeCredentialsManager( &g_pCredManager,
                                                     g_ConfigDirectory);

                                                     */
        // Non-essential
        //LoadAccessToken();
        
        status = liba::InitializeTaskArbiter(threadCount);

        // Load Entity Authentication  - ORDER MATTERS
        status = LoadEntity();
         if(status != ret::A_OK)
            alog::Log(Logger::ERROR, "Failed to load Entity");

        status = LoadPhraseToken();
        if(status != ret::A_OK)
            alog::Log(Logger::ERROR, "Failed to load phrase token");

        // Try to load a master key if we have one
        if(LoadMasterKey() == ret::A_OK) {  // don't set it equal to status, because if this fails
                                            // it's really not that important, we can have the user
                                            // go ahead and enter it.
        }

        AccessToken at;
        pClient->GetCredentialsManager()->GetAccessTokenCopy(at);
        status = liba::InitializeTaskManager( &g_pTaskManager,
                                              pClient->GetTentApp(),
                                              pClient->GetFileManager(),
                                              pClient->GetCredentialsManager(),
                                              at,
                                              g_Entity,
                                              g_TempDirectory,
                                              g_WorkingDirectory,
                                              g_ConfigDirectory);

        event::EventSystem::GetInstance()->Initialize();
        g_CallbackHandler.Initialize();


    }
    else {
        status = ret::A_FAIL_LOAD_APP_DATA;
        alog::Log(Logger::ERROR, "Failed to load app data");
    }

    if(status == ret::A_OK) { 
        g_bLibInitialized = true;
    }

    return status;
}

int ShutdownLibAttic(void (*callback)(int, void*))
{
    int status = ret::A_OK;

    // Shutdown threading first, ALWAYS
    status = liba::ShutdownTaskArbiter();
    //status = liba::ShutdownFileManager(&g_pFileManager);
    //status = liba::ShutdownCredentialsManager(&g_pCredManager);
    //status = liba::ShutdownAppInstance(pClient->GetTentApp());
    status = liba::ShutdownTaskManager(&g_pTaskManager);

    event::ShutdownEventSystem();
    //g_pFileManager = NULL;
    //g_pCredManager = NULL;
    //g_pApp = NULL;
    g_pTaskManager = NULL;

    if(callback)
        callback(status, NULL);

    alog::ShutdownLogging();

    g_bLibInitialized = false;
    return status;
}
// Move these two methods to apputils
static TentApp* g_pApp = NULL;
int StartupAppInstance( const char* szAppName, 
                        const char* szAppDescription, 
                        const char* szUrl, 
                        const char* szIcon, 
                        char* redirectUris[], 
                        unsigned int uriCount, 
                        char* scopes[], 
                        unsigned int scopeCount)
{
    int status = ret::A_OK;

    if(!g_pApp)
        g_pApp = new TentApp();                                                

    std::vector<std::string> uris;
    for(unsigned int i=0; i < uriCount; i++)
        uris.push_back(redirectUris[i]);

    std::vector<std::string> scopesVec;
    for(unsigned int i=0;i<scopeCount; i++)
        scopesVec.push_back(scopes[i]);

    status = app::StartupAppInstance( *g_pApp,
                                      szAppName,
                                      szAppDescription,
                                      szUrl,
                                      szIcon,
                                      uris,
                                      scopesVec);
    return status;
}

int RegisterApp(const char* szEntityUrl, const char* szConfigDirectory) {
    if(!szConfigDirectory) return ret::A_FAIL_INVALID_PTR;
    if(!szEntityUrl) return ret::A_FAIL_INVALID_PTR;
    if(!g_pApp) return ret::A_FAIL_INVALID_APP_INSTANCE;

    int status = app::RegisterApp(*g_pApp, szEntityUrl, szConfigDirectory);
/*
    std::string config_dir = szConfigDirectory;
    fs::CreateDirectory(config_dir);

    std::string postpath, entityurl;
    entityurl = szEntityUrl;
    utils::CheckUrlAndAppendTrailingSlash(entityurl);
    postpath += GetEntityApiRoot(entityurl.c_str());
    utils::CheckUrlAndAppendTrailingSlash(postpath);
    postpath += "apps";

    int status = ret::A_OK;
    std::string serialized;
    if(jsn::SerializeObject(g_pApp, serialized)) {
        Response response;
        status = netlib::HttpPost( postpath, 
                                   NULL,
                                   serialized,
                                   NULL,
                                   response);

        std::cout<< " CODE : " << response.code << std::endl;
        std::cout<< " BODY : " << response.body << std::endl;
        // Deserialize new data into app
        if(response.code == 200) {
            if(jsn::DeserializeObject(g_pApp, response.body)) {
                //SaveAppToFile();
                std::string savepath = config_dir;
                utils::CheckUrlAndAppendTrailingSlash(savepath);
                savepath.append(cnst::g_szAppDataName);
                status = g_pApp->SaveToFile(savepath);
            }
            else { 
                status = ret::A_FAIL_TO_DESERIALIZE_OBJECT;
            }
        }
        else {
            status = ret::A_FAIL_NON_200;
        }
    }
    else {
        status = ret::A_FAIL_TO_SERIALIZE_OBJECT;
    }
*/
    if(g_pApp) {
        delete g_pApp;
        g_pApp = NULL;
    }

    return status;
}

int RequestAppAuthorizationURL(const char* szEntityUrl)
{
    if(!pClient->GetTentApp()) return ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!szEntityUrl) return ret::A_FAIL_INVALID_CSTR;

    int status = app::RequestAppAuthorizationURL(*pClient->GetTentApp(), 
                                                 szEntityUrl, 
                                                 g_AuthorizationURL);
/*
    std::string apiroot;
    apiroot = GetEntityApiRoot(szEntityUrl);
//
    if(apiroot.empty())
        return ret::A_FAIL_EMPTY_STRING;


    UrlParams val;
    //val.AddValue(std::string("client_id"), g_pApp->GetAppID());
    val.AddValue(std::string("client_id"), g_pApp->GetAppID());

    if(pClient->GetTentApp()->GetRedirectURIs()) {
        TentApp::RedirectVec* pUris = g_pApp->GetRedirectURIs();
        TentApp::RedirectVec::iterator itr = pUris->begin();

        for(;itr!=pUris->end();itr++) {
            val.AddValue(std::string("redirect_uri"), *itr);
        }
    }

    if(g_pApp->GetScopes()) {
        TentApp::ScopeVec* pScopes = g_pApp->GetScopes();
        TentApp::ScopeVec::iterator itr = pScopes->begin();

        for(;itr!=pScopes->end();itr++) {
            val.AddValue(std::string("scope"), *itr);
        }
    }

    val.AddValue("tent_profile_info_types", "all");
    val.AddValue("tent_post_types", "all");
    //val.AddValue("tent_post_types", "https://tent.io/types/posts/status/v0.1.0");

    g_AuthorizationURL.clear();
    g_AuthorizationURL.append(apiroot);

    utils::CheckUrlAndAppendTrailingSlash(g_AuthorizationURL);

    g_AuthorizationURL.append("oauth/authorize");

    std::string params;
    val.SerializeToString(params);

    // TODO:: encode these parameters

    g_AuthorizationURL.append(params);
    */

    return ret::A_OK;
}

// Move to apputils
int RequestUserAuthorizationDetails(const char* szEntityUrl,
                                    const char* szCode, 
                                    const char* szConfigDirectory)
{
    int status = ret::A_OK;
    SetConfigDirectory(szConfigDirectory);// depricated

    if(!g_pApp)
        g_pApp = new TentApp();                                                

    status = app::RequestUserAuthorizationDetails(*g_pApp, 
                                                  szEntityUrl, 
                                                  szCode, 
                                                  szConfigDirectory);

    /*
    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    status = g_pApp->LoadFromFile(szSavePath);
    if(status != ret::A_OK)
        return status;

    if(!g_pApp)             return ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!szCode)             return ret::A_FAIL_INVALID_CSTR;
    if(!szEntityUrl)        return ret::A_FAIL_INVALID_CSTR;
    if(!szConfigDirectory)  return ret::A_FAIL_INVALID_CSTR;

    std::string apiroot;
    apiroot = GetEntityApiRoot(szEntityUrl);

    if(apiroot.empty())
        return ret::A_FAIL_EMPTY_STRING;

    // Build redirect code
    RedirectCode rcode;
    rcode.SetCode(std::string(szCode));
    rcode.SetTokenType(std::string("mac"));

    std::string path(apiroot);
    utils::CheckUrlAndAppendTrailingSlash(path);
    path.append("apps/");
    path.append(pClient->GetTentApp()->GetAppID());
    path.append("/authorizations");

    // serialize RedirectCode
    std::string serialized;
    if(!jsn::SerializeObject(&rcode, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    Response response;

    AccessToken at;
    at.m_MacAlgorithm = g_pApp->GetMacAlgorithm();
    at.m_AccessToken = g_pApp->GetMacKeyID();
    at.m_MacKey = g_pApp->GetMacKey();

    netlib::HttpPost(path, NULL, serialized, &at, response);

    if(response.code == 200) {
        AccessToken at;
        status = liba::DeserializeIntoAccessToken(response.body, at);
        if(status == ret::A_OK) {
            status = liba::WriteOutAccessToken(at, g_ConfigDirectory);
        }
    }
    else {
        std::ostringstream error;
        error << "Non 200 repsonse in RequestUserAuthorizationDetails" << std::endl;
        error << "Code : " << response.code << std::endl;
        error << "Body : " << response.body << std::endl;
        event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);

        status = ret::A_FAIL_NON_200;
    }
    */

    if(g_pApp) {
        delete g_pApp;
        g_pApp = NULL;
    }
        

    return status;
}

const char* GetAuthorizationURL()
{
    return g_AuthorizationURL.c_str();
}




/*
int LoadAccessToken() { // Depricated
    int status = g_pCredManager->LoadAccessToken();
    if(status == ret::A_OK) {
        g_pCredManager->GetAccessTokenCopy(g_AccessToken);
    }
    
    return status; 
}
*/
/*
int SaveAppToFile() // Depricated
{
    if(!pClient->GetTentApp()) return ret::A_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    return pClient->GetTentApp()->SaveToFile(szSavePath);
}
*/

/*
int LoadAppFromFile() { // Depricated
    int status = ret::A_OK;

    if(!pClient->GetTentApp())
        pClient->GetTentApp() = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    status = pClient->GetTentApp()->LoadFromFile(szSavePath);

    if(status == ret::A_OK) {
        std::string buffer;
        jsn::SerializeObject(pClient->GetTentApp(), buffer);
        //std::cout<<" BUFFER : " << buffer << std::endl;
    }

    return status;
}
*/

int PushFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == ret::A_OK){
        std::string filepath(szFilePath);
        event::RaiseEvent(event::Event::REQUEST_PUSH, filepath, NULL);
    }
    return status;
}

int PullFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == ret::A_OK){
        std::string filepath(szFilePath);
        event::RaiseEvent(event::Event::REQUEST_PULL, filepath, NULL);
    }

    return ret::A_OK;
}

int DeleteFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == ret::A_OK) {
        std::string filepath(szFilePath);
        event::RaiseEvent(event::Event::REQUEST_DELETE, filepath, NULL);
    }

    return status;
}

int SyncFiles(void) {
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->SyncFiles(NULL);

    return status;
}

int PollFiles(void) {
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->PollFiles(NULL);

    return status;
}

int GetMasterKeyFromProfile(std::string& out) {
    std::cout<<" Getting master key from profile ... " << std::endl;
    int status = ret::A_OK;

    Profile* prof = g_Entity.GetFrontProfile();
    if(prof) {
        AtticProfileInfo* atpi = prof->GetAtticInfo();
        if(atpi)
            atpi->GetMasterKey(out);
    }
    else
        status = ret::A_FAIL_INVALID_PROFILE;

    return status;
}

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv) {
    int status = ret::A_OK;

    if(!phraseKey.empty()) {
        Credentials cred;
        cred.SetKey(phraseKey);
        cred.SetIv(iv);

        std::string encmk; // Encrypted Master Key
        status = GetMasterKeyFromProfile(encmk);

        if(status == ret::A_OK) {
            if(!encmk.empty()) {
                // Attempt to Decrypt Master Key
                std::string out;
                status = crypto::DecryptStringCFB(encmk, cred, out);

                if(status == ret::A_OK) {
                    if(out.size() >= 8) {
                        // Check sentinel bytes
                        std::string sentone, senttwo;
                        sentone = out.substr(0, 4);
                        senttwo = out.substr(4, 4);
                        
                        if(sentone == senttwo) {
                            // extract actual key apart from sentinel bytes
                            std::string keyActual;
                            keyActual = out.substr(8);

                            // Shove this somewhere
                            MasterKey masterKey;
                            masterKey.SetMasterKey(keyActual);

                            // Insert Into Credentials Manager
                            pClient->GetCredentialsManager()->SetMasterKey(masterKey);

                            g_Pt.SetPhraseKey(phraseKey);
                            SavePhraseToken(g_Pt);
                            g_bEnteredPassphrase = true;
                        }
                        else {
                            status = ret::A_FAIL_SENTINEL_MISMATCH;
                            alog::Log(Logger::ERROR, " Failed to decrypt master key : ", status);
                        }
                    }
                    else {
                        status = ret::A_FAIL_OTHER;
                    }
                }
            }
            else {
                status = ret::A_FAIL_INVALID_MASTERKEY;
            }
        }
    }
    else {
        status = ret::A_FAIL_EMPTY_PASSPHRASE;
    }

    std::cout << " Decrypt Master Key status : " << status << std::endl;
    return status;
}

int RegisterPassphrase(const char* szPass, bool override) {
    int status = IsLibInitialized(false);

    if(status == ret::A_OK) {
        // TODO :: probably should check if a passphrase already exists
        // TODO :: probably should include static test case to detect if the passphrase entered was wrong.
        //          - at the begining of the master key append 4 random bytes repeated twice,
        //          - check the first 4 against the latter 4 and if they are the same you entered
        //            the passphrase in correctly.
        //          - obviously skip the first 8 bytes when getting the master key
        //TODO :: figure out way to check if there is a passphrase already set, then warn against overwrite
        status = ret::A_FAIL_REGISTER_PASSPHRASE;

        if(!g_Entity.HasAtticProfileMasterKey() || override) {
            // Register a new passphrase.
            std::string key;
            // Enter passphrase to generate key.
            pClient->GetCredentialsManager()->GenerateMasterKey(key); // Generate random master key

            std::string recoverykey;

            std::cout<<" MASTER KEY : " << key << std::endl;
            status = pass::RegisterPassphraseWithAttic(std::string(szPass), 
                                                       key, // master key
                                                       pClient->GetCredentialsManager(),
                                                       g_Entity,
                                                       g_Pt,
                                                       recoverykey);
             if(status == ret::A_OK) {
                SavePhraseToken(g_Pt);
                std::cout<<" RAISING EVENT : " << recoverykey << std::endl;
                event::RaiseEvent(event::Event::RECOVERY_KEY, recoverykey, NULL);
             }
        }

        if(status == ret::A_OK)
            EnterPassphrase(szPass); // Load phrase token, and write out to ent file
    }

    return status;
}

int EnterPassphrase(const char* szPass) {
    int status = IsLibInitialized(false);

    if(status == ret::A_OK) {
        // Enter the passphrase and generate phrase token
        // TODO :: when entering the passphrase always check against the master key with sentinel
        //         if the first 8 bytes don't match (4 bytes == next 4 bytes) then the user entered
        //         the wrong passphrase.
        //         so put a check for that everytime the user enteres the passphrase
        
        // Check for correct passphrase
        // Get Information from entity

        // Force download entity
        LoadEntity(true);

        std::string salt;
        g_Pt.GetSalt(salt);

        std::cout<<" entering pass ... " << std::endl;
        std::string phraseKey;
        status = pClient->GetCredentialsManager()->EnterPassphrase(szPass, salt, phraseKey); // Enter passphrase to generate key.

        //std::cout<<" PHRASE KEY : " << phraseKey << std::endl;
        //std::cout<<" SALT : " << salt << std::endl;

        if(status == ret::A_OK) {
            status = DecryptMasterKey(phraseKey, salt);

            std::cout<<" DECRYPT STATUS : " << status << std::endl;
            if(status == ret::A_OK) {
                // Reload phrase token
                //LoadPhraseToken();
                // Load Master Key
                status = LoadMasterKey();
                std::cout<< " LOAD MASTER KEY STATUS : " << status << std::endl;
            }
        }
    }

    return status;
}

int ChangePassphrase(const char* szOld, const char* szNew) {
    int status = IsLibInitialized(false);

    if(status == ret::A_OK) {
        status = EnterPassphrase(szOld);

        if(status == ret::A_OK) {
            // Get the master key
            MasterKey mk;
            pClient->GetCredentialsManager()->GetMasterKeyCopy(mk);

            // Register new passphrase with attic
            std::string key;
            mk.GetMasterKey(key);

            std::string recoverykey;
            status = pass::RegisterPassphraseWithAttic( std::string(szNew), 
                                                        key, // master key
                                                        pClient->GetCredentialsManager(),
                                                        g_Entity,
                                                        g_Pt,
                                                        recoverykey);
            if(status == ret::A_OK){
                SavePhraseToken(g_Pt);
                event::RaiseEvent(event::Event::RECOVERY_KEY, recoverykey, NULL);
            }
        }
    }

    return status;
}

int EnterRecoveryKey(const char* szRecovery) {

    int status = IsLibInitialized(false);
    if(!szRecovery) status = ret::A_FAIL_INVALID_CSTR;
    if(status == ret::A_OK) {

        std::string recovery_key(szRecovery);
        status = LoadEntity(true);
        if(status == ret::A_OK) {
            std::string masterkey;
            status = pass::EnterRecoveryKey(recovery_key, pClient->GetCredentialsManager(), g_Entity, masterkey);
            std::cout<<" MASTER KEY : " << masterkey << std::endl;
            if(status == ret::A_OK) {
                std::string temppass;
                utils::GenerateRandomString(temppass, 16);
                std::string new_recovery_key;
                status = pass::RegisterPassphraseWithAttic(temppass, 
                                                           masterkey,
                                                           pClient->GetCredentialsManager(),
                                                           g_Entity,
                                                           g_Pt,
                                                           new_recovery_key);

                if(status == ret::A_OK) {
                    SavePhraseToken(g_Pt);
                    //status = EnterPassphrase(temppass); // Load phrase token, and write out to ent file
                    event::RaiseEvent(event::Event::TEMPORARY_PASS, temppass, NULL);
                }
            }
        }
    }

    return status;
}

const char** GetQuestionList() {
    static const char* t[]={
                            {"What is your spirit animal?"}, 
                            {"What is your favorite color?"},
                            {"What is your color?"}
                           };
    return t;
}

int RegisterQuestionAnswerKey(const char* q1, 
                              const char* q2, 
                              const char* q3, 
                              const char* a1, 
                              const char* a2, 
                              const char* a3)
{
    int status = LoadEntity(true);
    if(status == ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(pClient->GetCredentialsManager()) {
            MasterKey mk;
            std::string masterkey;
            pClient->GetCredentialsManager()->GetMasterKeyCopy(mk);
            mk.GetMasterKey(masterkey);
            status = pass::RegisterRecoveryQuestionsWithAttic(masterkey, 
                                                              pClient->GetCredentialsManager(), 
                                                              g_Entity, 
                                                              questionOne,
                                                              questionTwo,
                                                              questionThree,
                                                              answerOne,
                                                              answerTwo,
                                                              answerThree);
        }
        else {
            status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
        }
    }

    return status;
}

int EnterQuestionAnswerKey(const char* q1, 
                           const char* q2, 
                           const char* q3, 
                           const char* a1, 
                           const char* a2, 
                           const char* a3)
{
    int status = LoadEntity(true);
    if(status == ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(pClient->GetCredentialsManager()) {
            std::string mkOut;
            status = pass::EnterRecoveryQuestions(pClient->GetCredentialsManager(),
                                            g_Entity,
                                            questionOne,
                                            questionTwo,
                                            questionThree,
                                            answerOne,
                                            answerTwo,
                                            answerThree,
                                            mkOut);
            if(status == ret::A_OK) {
                std::string temppass;
                utils::GenerateRandomString(temppass, 16);
                std::string new_recovery_key;
                status = pass::RegisterPassphraseWithAttic(temppass, 
                                                           mkOut,
                                                           pClient->GetCredentialsManager(),
                                                           g_Entity,
                                                           g_Pt,
                                                           new_recovery_key);

                if(status == ret::A_OK) {
                    SavePhraseToken(g_Pt);
                    event::RaiseEvent(event::Event::TEMPORARY_PASS, temppass, NULL);
                }
            }
        }
        else {
            status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
        }
    }
    return status;
}

int LoadPhraseToken() {
    std::string ptpath;
    GetPhraseTokenFilepath(ptpath);

    int status = g_Pt.LoadFromFile(ptpath);
    if(status != ret::A_OK) {
        // couldn't load from file (non existant)
        // Extract partial info
        // Extract Info from entity
        Profile* prof = g_Entity.GetFrontProfile();
        if(prof) {
            AtticProfileInfo* atpi = prof->GetAtticInfo();
            if(atpi) {
                std::string salt;
                atpi->GetSalt(salt);
                g_Pt.SetSalt(salt);

                std::string iv;
                atpi->GetIv(iv);
                g_Pt.SetIv(iv);

                std::string key;
                atpi->GetMasterKey(key);
                g_Pt.SetDirtyKey(key);

                // Save token to file
                SavePhraseToken(g_Pt);
                g_bEnteredPassphrase = false;

                //status = ret::A_FAIL_INVALID_PHRASE_TOKEN;
                status = g_Pt.LoadFromFile(ptpath);
            }
        }
        else {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }
    else {
        if(g_Pt.IsPhraseKeyEmpty())
            g_bEnteredPassphrase = false;
        else
            g_bEnteredPassphrase = true;
    }

    std::string pk, dk;
    g_Pt.GetDirtyKey(dk);
    g_Pt.GetPhraseKey(pk);
    return status; 
}

int SavePhraseToken(PhraseToken& pt) {
    std::string ptpath;
    GetPhraseTokenFilepath(ptpath);
    return g_Pt.SaveToFile(ptpath);
}

int PhraseStatus() {
    int status = ret::A_OK;

    if(!g_bEnteredPassphrase)
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;

    return status;
}

int LoadMasterKey() {
    int status = ret::A_OK;
    // Check for valid phrase token
    if(g_Pt.IsPhraseKeyEmpty()) {
        // "Enter Password"
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }
    else {
        std::string phraseKey, salt;
        g_Pt.GetPhraseKey(phraseKey);
        g_Pt.GetSalt(salt);

        status = DecryptMasterKey(phraseKey, salt);
    }   

    return status;
}

void GetPhraseTokenFilepath(std::string& out) {
    out += g_ConfigDirectory;
    out += "/";
    out += cnst::g_szPhraseTokenName;
}

void GetEntityFilepath(std::string& out) {
    out += g_ConfigDirectory;
    out += "/";
    out += cnst::g_szEntityName;
}

int LoadEntity(bool override) {
    std::string entpath;
    GetEntityFilepath(entpath);
    int status = g_Entity.LoadFromFile(entpath);

    // Check for master key
    if(!g_Entity.HasAtticProfileMasterKey())
        status = ret::A_FAIL_INVALID_MASTERKEY;
    
    if(status != ret::A_OK || override){
        if(override)
            g_Entity.Reset();

        // Load Entity
        AccessToken at;
        pClient->GetCredentialsManager()->GetAccessTokenCopy(at);

        status = g_Entity.Discover(g_EntityUrl, &at);

        if(status == ret::A_OK)
            g_Entity.WriteToFile(entpath);
        else
            alog::Log(Logger::DEBUG, "LoadEntity failed discovery : " + g_EntityUrl);
    }
    return status;
}

int GetPhraseStatus() {

    return ret::A_OK;
}

int SaveEntity() {

    return ret::A_OK;
}

int SetWorkingDirectory(const char* szDir) {
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_WorkingDirectory.clear();
    g_WorkingDirectory.append(szDir);

    fs::CreateDirectory(g_WorkingDirectory);

    return ret::A_OK;
}

int SetConfigDirectory(const char* szDir) {
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_ConfigDirectory.clear();
    g_ConfigDirectory.append(szDir);

    fs::CreateDirectory(g_ConfigDirectory);

    return ret::A_OK;
}

int SetTempDirectory(const char* szDir) {
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_TempDirectory.clear();
    g_TempDirectory.append(szDir);

    fs::CreateDirectory(g_TempDirectory);

    return ret::A_OK;
}

int SetEntityUrl(const char* szUrl) {
    if(!szUrl)
        return ret::A_FAIL_INVALID_CSTR;

    g_EntityUrl.append(szUrl);
    g_Entity.SetEntityUrl(szUrl);
    std::string test;
    g_Entity.GetEntityUrl(test);

    return ret::A_OK;
}

int IsLibInitialized(bool checkPassphrase) {
    int status = ret::A_OK;

    if(checkPassphrase && g_Pt.IsPhraseKeyEmpty()) {
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }

    if(!pClient->GetTentApp()) status = ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!pClient->GetCredentialsManager()) status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    if(!pClient->GetFileManager()) status = ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE; 
    //if(!g_bLibInitialized) status = ret::A_FAIL_LIB_INIT;

    return status;
}

const char* GetEntityApiRoot(const char* szEntityUrl) {
    if(!szEntityUrl) return NULL;

    Entity out;
    std::string entityurl(szEntityUrl);
    utils::CheckUrlAndAppendTrailingSlash(entityurl);
    int status = client::Discover(entityurl, NULL, out);

    std::string apiroot;
    if(status == ret::A_OK)
        out.GetApiRoot(apiroot);
    else
        alog::Log(Logger::ERROR, "GentEntityApiRoot, libattic.cpp", status);

    return apiroot.c_str();
}

const char* GetWorkingDirectory() { return g_WorkingDirectory.c_str(); }
const char* GetConfigDirectory() { return g_ConfigDirectory.c_str(); }
const char* GetEntityUrl() { return g_EntityUrl.c_str(); }

int GetFileList(void(*callback)(int, char**, int, int)) {
    int status = ret::A_OK;
    if(!callback)
        status = ret::A_FAIL_INVALID_PTR;

    if(status == ret::A_OK) {
        status = IsLibInitialized();

        if(status == ret::A_OK)
            status = g_pTaskManager->QueryManifest(callback);
    }

    return status;
}

int FreeFileList(char** pList, int stride) {
    if(pList) {
        char* p = *pList;
        int count = 0;
        for(int i=0; i< stride; i++) {
            if(pList[i]) {
                delete[] pList[i];
                pList[i] = NULL;
            }
        }
        delete[] pList;
        pList = NULL;
    }
}

void RegisterForPullNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::PULL, callback);
}

void RegisterForPushNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::PUSH, callback);
}

void RegisterForUploadSpeedNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::UPLOAD_SPEED, callback);
}

void RegisterForDownloadSpeedNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::DOWNLOAD_SPEED, callback);
}

void RegisterForErrorNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::ERROR_NOTIFY, callback);
}

void RegisterForRecoveryKeyNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::RECOVERY_KEY, callback);
}

void RegisterForTemporaryKeyNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::TEMPORARY_PASS, callback);
}

void RegisterForPauseResumeNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(event::Event::PAUSE_RESUME_NOTIFY, callback);
}

int Pause(void) { 
    event::RaiseEvent(event::Event::PAUSE, "", NULL);
}

int Resume(void) {
    event::RaiseEvent(event::Event::RESUME, "", NULL);
}



