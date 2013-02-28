#include "libattic.h"

#include <fstream>
#include <string>

#include "entitymanager.h"
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
#include "conoperations.h"

#include "libatticutils.h"
#include "log.h"

#include "taskmanager.h"

#include <cbase64.h>
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

static TentApp*             g_pApp = NULL;
static FileManager*         g_pFileManager = NULL;
static CredentialsManager*  g_pCredManager = NULL;
static EntityManager*       g_pEntityManager = NULL;
static TaskManager*         g_pTaskManager = NULL;

static AccessToken          g_AccessToken;
//static TaskArbiter g_Arb;

// Directories
static std::string g_WorkingDirectory;
static std::string g_ConfigDirectory;
static std::string g_TempDirectory;

// var
static std::string g_EntityUrl;
static std::string g_AuthorizationURL;
static PhraseToken  g_Pt;
static Entity g_Entity;

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

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey);

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv);

int IsLibInitialized(bool checkPassphrase = true);


//TODO TESTING METHODS REMOVE
FileManager* GetFileManager() { return g_pFileManager; }

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

    // Initialize logging
    alog::InitializeLogging(szLogDirectory);
    alog::Log(Logger::DEBUG, "Init");

    status = LoadAppFromFile();
    if(status == ret::A_OK)
    {
        // Essential
        status = SetEntityUrl(szEntityURL);

        if(status != ret::A_OK) 
            alog::Log(Logger::ERROR, "Failed to set entity url");

        status = liba::InitializeFileManager( &g_pFileManager,
                                              g_ConfigDirectory,
                                              g_WorkingDirectory);

        status = liba::InitializeCredentialsManager( &g_pCredManager,
                                                     g_ConfigDirectory);

        status = liba::InitializeEntityManager( &g_pEntityManager );
        // Non-essential
        LoadAccessToken();
        
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
        g_pCredManager->GetAccessTokenCopy(at);
        status = liba::InitializeTaskManager( &g_pTaskManager,
                                              g_pApp,
                                              g_pFileManager,
                                              g_pCredManager,
                                              at,
                                              g_Entity,
                                              g_TempDirectory,
                                              g_WorkingDirectory,
                                              g_ConfigDirectory);


    }
    else {
        status = ret::A_FAIL_LOAD_APP_DATA;
        alog::Log(Logger::ERROR, "Failed to load app data");
    }

    if(status == ret::A_OK)
        g_bLibInitialized = true;
    return status;
}

int ShutdownLibAttic(void (*callback)(int, void*))
{
    int status = ret::A_OK;

    // Shutdown threading first, ALWAYS
    status = liba::ShutdownTaskArbiter();
    status = liba::ShutdownFileManager(&g_pFileManager);
    status = liba::ShutdownCredentialsManager(&g_pCredManager);
    status = liba::ShutdownEntityManager(&g_pEntityManager);
    status = liba::ShutdownAppInstance(&g_pApp);
    status = liba::ShutdownTaskManager(&g_pTaskManager);

    g_pFileManager = NULL;
    g_pCredManager = NULL;
    g_pEntityManager = NULL;
    g_pApp = NULL;
    g_pTaskManager = NULL;

    if(callback)
        callback(status, NULL);

    alog::ShutdownLogging();
    g_bLibInitialized = false;
    return status;
}

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

int RegisterApp(const char* szEntityUrl, const char* szConfigDirectory)
{
    if(!szConfigDirectory) return ret::A_FAIL_INVALID_PTR;
    if(!szEntityUrl) return ret::A_FAIL_INVALID_PTR;
    if(!g_pApp) return ret::A_FAIL_INVALID_APP_INSTANCE;

    g_ConfigDirectory.clear();
    g_ConfigDirectory += (szConfigDirectory);

    std::string postpath;
    postpath += GetEntityApiRoot(szEntityUrl);
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
                SaveAppToFile();
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
    return status;
}

int RequestAppAuthorizationURL(const char* szEntityUrl)
{
    if(!g_pApp) return ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!szEntityUrl) return ret::A_FAIL_INVALID_CSTR;

    std::string apiroot;
    apiroot = GetEntityApiRoot(szEntityUrl);

    if(apiroot.empty())
        return ret::A_FAIL_EMPTY_STRING;


    UrlParams val;
    val.AddValue(std::string("client_id"), g_pApp->GetAppID());

    if(g_pApp->GetRedirectURIs()) {
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

    return ret::A_OK;
}

const char* GetAuthorizationURL()
{
    return g_AuthorizationURL.c_str();
}


int RequestUserAuthorizationDetails( const char* szEntityUrl,
                                     const char* szCode, 
                                     const char* szConfigDirectory)
{
    SetConfigDirectory(szConfigDirectory);
    LoadAppFromFile();

    if(!g_pApp) return ret::A_FAIL_INVALID_APP_INSTANCE;
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
    path.append(g_pApp->GetAppID());
    path.append("/authorizations");

    // serialize RedirectCode
    std::string serialized;
    if(!jsn::SerializeObject(&rcode, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    int status = ret::A_OK;
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
        status = ret::A_FAIL_NON_200;
    }

    return status;
}

int LoadAccessToken()
{
    int status = g_pCredManager->LoadAccessToken();

    if(status == ret::A_OK) {
        g_pCredManager->GetAccessTokenCopy(g_AccessToken);
    }
    
    return status; 
}

int SaveAppToFile()
{
    if(!g_pApp) return ret::A_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    return g_pApp->SaveToFile(szSavePath);
}

int LoadAppFromFile()
{
    int status = ret::A_OK;

    if(!g_pApp)
        g_pApp = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    status = g_pApp->LoadFromFile(szSavePath);

    if(status == ret::A_OK)
    {
        std::string buffer;
        jsn::SerializeObject(g_pApp, buffer);
        //std::cout<<" BUFFER : " << buffer << std::endl;
    }

    return status;
}

int PushFile(const char* szFilePath, void (*callback)(int, void*) )
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->UploadFile(szFilePath, callback);

    return status;
}

int PullFile(const char* szFilePath, void (*callback)(int, void*))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->DownloadFile(szFilePath, callback);

    return ret::A_OK;
}

int PullAllFiles(void (*callback)(int, void*))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->DownloadAllFiles(callback);

    return status;
}

int SyncFiles(void (*callback)(int, void*))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->SyncFiles(callback);

    return status;
}

int DeleteFile(const char* szFilePath, void (*callback)(int, void*) )
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->DeleteFile(szFilePath, callback);

    return status;
}

int DeleteAllPosts(void (*callback)(int, void*))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->DeleteAllPosts(callback);

    return status;
}

int GetCurrentTasks(void (*callback)(char* pArr, int count))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
    {


    }
 
    return status;
}

int ChangePassphrase(const char* szOld, const char* szNew)
{
    int status = IsLibInitialized(false);

    if(status == ret::A_OK)
    {
        // TODO :: make this a task
        // TODO :: this should be a task
        //         pull all files
        //         decrypt them with the old key
        //         re-encrypt them with the new key
        status = EnterPassphrase(szOld);

        if(status == ret::A_OK)
        {
            // Get the master key
            MasterKey mk;
            g_pCredManager->GetMasterKeyCopy(mk);

            // Register new passphrase with attic
            std::string key;
            mk.GetMasterKey(key);

            status = RegisterPassphraseWithAttic(szNew, key);
        }
    }

    return status;
}

int GetMasterKeyFromProfile(std::string& out)
{
    int status = ret::A_OK;

    Profile* prof = g_Entity.GetFrontProfile();
    if(prof) {
        AtticProfileInfo* atpi = prof->GetAtticInfo();
        if(atpi)
            atpi->GetMasterKey(out);
    }
    else
        status = ret::A_FAIL_INVALID_PTR;

    return status;
}

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv)
{
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
                Crypto crypto;
                status = crypto.DecryptStringCFB(encmk, cred, out);

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
                            g_pCredManager->SetMasterKey(masterKey);

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
    return status;
}

// Master Key
int EnterPassphrase(const char* szPass)
{
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

        std::string phraseKey;

        status = g_pCredManager->EnterPassphrase(szPass, salt, phraseKey); // Enter passphrase to generate key.

        std::cout<<" PHRASE KEY : " << phraseKey << std::endl;
        std::cout<<" SALT : " << salt << std::endl;

        if(status == ret::A_OK) {
            status = DecryptMasterKey(phraseKey, salt);

            if(status == ret::A_OK) {
                // Reload phrase token
                //LoadPhraseToken();
                // Load Master Key
                status = LoadMasterKey();
            }
        }
    }

    return status;
}

int ConstructMasterKey(const std::string& masterkey, MasterKey& out)
{
    int status = ret::A_OK;

    // Enter passphrase to generate key.
    g_pCredManager->RegisterPassphrase(masterkey, g_Pt); // This generates a random salt
                                                         // Sets Phrase key
    g_pCredManager->CreateMasterKeyWithPass(out, masterkey); // Create Master Key with given pass
    g_pCredManager->SetMasterKey(out);

    return status;
}

int EncryptKeyWithPassphrase( const std::string& key, 
                              const std::string& phrasekey, 
                              const std::string& salt,
                              std::string& keyOut)
{
    int status = ret::A_OK;

    Crypto crypto;

    // encryption credentials
    Credentials enc;
    enc.SetKey(phrasekey);
    enc.SetIv(salt);

    //std::cout<< "Encrypting key with passphrase key : " << phrasekey << std::endl;
    //std::cout<< "Using iv : " << salt << std::endl;
    // Encrypt MasterKey with passphrase key
    std::string out;
    crypto.EncryptStringCFB(key, enc, keyOut);
    
    return status;
} 

// TODO :: this can be abstracted somewhere
int RegisterPassphraseProfilePost( const std::string& encryptedKey, const std::string& salt)
{
    int status = ret::A_OK;

    // Create Profile post for 
    AtticProfileInfo* pAtticProf = new AtticProfileInfo();
    // MasterKey with sentinel and Salt
    pAtticProf->SetMasterKey(encryptedKey);
    pAtticProf->SetSalt(salt);

    // Save and post
    std::string output;
    jsn::SerializeObject(pAtticProf, output);

    std::string url;
    g_Entity.GetFrontProfileUrl(url);

    AccessToken at;
    if(g_pCredManager) {
        g_pCredManager->GetAccessTokenCopy(at);
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }

    if(status == ret::A_OK) {
        // TODO :: add the type as url params and just pass the attic profile type
        // UrlParams params
        std::string hold(cnst::g_szAtticProfileType);
        hold = netlib::UriEncode(hold);

        url.append("/");
        url.append(hold);

        Response resp;
        netlib::HttpPut(url, NULL, output, &at, resp);

        if(resp.code != 200) {
            status = ret::A_FAIL_NON_200;
            alog::Log(Logger::DEBUG, "RegisterPassphraseProfilePost : \n" +
                      resp.CodeAsString() +
                      resp.body);
        }
    }

    return status;
}

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey)
{
    int status = ret::A_OK;
    // Have the passphrase, and the master key
    // generate random iv and salt
    // Inward facing method
    // Register a new passphrase.

    /*
    std::string gptphrasekey, gptdirtykey;
    g_Pt.GetPhraseKey(gptphrasekey);
    g_Pt.GetDirtyKey(gptdirtykey);
    std::cout<<" Phrase key : " << gptphrasekey << std::endl;
    std::cout<<" Dirty key : " << gptdirtykey << std::endl;
    */

    MasterKey mk;
    status = ConstructMasterKey(masterkey, mk); // also generates salt, inserts into 
                                                // phrase token

    std::string genmk; mk.GetMasterKey(genmk);

    if(status == ret::A_OK) {
        // Insert sentinel value
        mk.InsertSentinelIntoMasterKey();
        // Get Salt
        std::string salt;
        g_Pt.GetSalt(salt);  // Phrase Token
        // Get Dirty Master Key (un-encrypted master key with sentinel values)
        std::string dirtykey;
        mk.GetMasterKeyWithSentinel(dirtykey);
        g_Pt.SetDirtyKey(dirtykey); // Set Phrase Token
        // Enter passphrase to generate key.
        std::string phraseKey;
        status = g_pCredManager->EnterPassphrase(pass, salt, phraseKey);
        if(status == ret::A_OK) {
            g_Pt.SetPhraseKey(phraseKey);
            // Setup passphrase cred to encrypt master key
            std::string encryptedkey;
            status = EncryptKeyWithPassphrase( dirtykey, 
                                               phraseKey, 
                                               salt, 
                                               encryptedkey);
            if(status == ret::A_OK) {
                status = RegisterPassphraseProfilePost( encryptedkey, 
                                                        salt);
                if(status == ret::A_OK)
                    SavePhraseToken(g_Pt);
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
       
    return status; 
}

int RegisterPassphrase(const char* szPass, bool override)
{
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
            std::string mk;
            // Enter passphrase to generate key.
            g_pCredManager->GenerateMasterKey(mk); // Generate random master key

            status = RegisterPassphraseWithAttic(szPass, mk);
        }

        if(status == ret::A_OK)
            EnterPassphrase(szPass); // Load phrase token, and write out to ent file
    }

    return status;
}

int LoadPhraseToken()
{
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

int SavePhraseToken(PhraseToken& pt)
{
    std::string ptpath;
    GetPhraseTokenFilepath(ptpath);
    return g_Pt.SaveToFile(ptpath);
}

int PhraseStatus()
{
    int status = ret::A_OK;

    if(!g_bEnteredPassphrase)
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;

    return status;
}

int LoadMasterKey()
{
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

void GetPhraseTokenFilepath(std::string& out)
{
    out += g_ConfigDirectory;
    out += "/";
    out += cnst::g_szPhraseTokenName;
}

void GetEntityFilepath(std::string& out)
{
    out += g_ConfigDirectory;
    out += "/";
    out += cnst::g_szEntityName;
}

int LoadEntity(bool override)
{
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
        g_pCredManager->GetAccessTokenCopy(at);

        status = g_pEntityManager->Discover(g_EntityUrl, at, g_Entity);

        if(status == ret::A_OK)
            g_Entity.WriteToFile(entpath);
        else
            alog::Log(Logger::DEBUG, "LoadEntity failed discovery : " + g_EntityUrl);
    }
    return status;
}

int GetPhraseStatus()
{

    return ret::A_OK;
}

int SaveEntity()
{

    return ret::A_OK;
}

int SetWorkingDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_WorkingDirectory.clear();
    g_WorkingDirectory.append(szDir);

    return ret::A_OK;
}

int SetConfigDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_ConfigDirectory.clear();
    g_ConfigDirectory.append(szDir);

    return ret::A_OK;
}

int SetTempDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_TempDirectory.clear();
    g_TempDirectory.append(szDir);

    return ret::A_OK;
}

int SetEntityUrl(const char* szUrl)
{
    if(!szUrl)
        return ret::A_FAIL_INVALID_CSTR;

    g_EntityUrl.append(szUrl);
    g_Entity.SetEntityUrl(szUrl);
    std::string test;
    g_Entity.GetEntityUrl(test);

    return ret::A_OK;
}

int IsLibInitialized(bool checkPassphrase)
{
    int status = ret::A_OK;

    if(checkPassphrase && g_Pt.IsPhraseKeyEmpty()) {
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }

    if(!g_pApp) status = ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!g_pCredManager) status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    if(!g_pFileManager) status = ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE; 
    //if(!g_bLibInitialized) status = ret::A_FAIL_LIB_INIT;

    return status;
}

const char* GetEntityApiRoot(const char* szEntityUrl)
{
    Entity out;
    int status = conops::Discover(szEntityUrl, out);

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

int GetFileList(void(*callback)(int, char**, int, int))
{
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->QueryManifest(callback);


    return status;
}

int FreeFileList(char** pList, int stride)
{
    if(pList)
    {
        char* p = *pList;
        int count = 0;
        for(int i=0; i< stride; i++)
        {
            if(pList[i])
            {
                delete[] pList[i];
                pList[i] = NULL;
            }

        }
        delete[] pList;
        pList = NULL;
    }
}

int GetActivePushTaskCount()
{
    int taskcount = -1;
    if(g_pTaskManager)
        taskcount = g_pTaskManager->TaskCount(Task::PUSH);
    return taskcount;
}

int GetActivePullTaskCount()
{
    int taskcount = -1;
    if(g_pTaskManager)
        taskcount = g_pTaskManager->TaskCount(Task::PULL);
    return taskcount;
}

int GetActiveUploadSpeed()
{
    int speed = -1;
    if(g_pTaskManager)
        speed = g_pTaskManager->GetActiveUploadSpeed();
    return speed; 
}

int GetActiveDownloadSpeed()
{
    int speed = -1;
    if(g_pTaskManager)
        g_pTaskManager->GetActiveDownloadSpeed();
    return speed; 
}
