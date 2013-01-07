#include "libattic.h"

#include <fstream>
#include <string>

#include "entitymanager.h"
#include "connectionmanager.h"
#include "credentialsmanager.h"
#include "filemanager.h"
#include "entity.h"

#include "utils.h"
#include "errorcodes.h"
#include "tentapp.h"
#include "jsonserializable.h"
#include "urlparams.h"
#include "post.h"

#include "taskfactory.h"
#include "taskarbiter.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "syncposttask.h"
#include "syncmanifesttask.h"

#include "constants.h"
#include "credentials.h"
#include "profile.h"
#include "conoperations.h"

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

static TaskArbiter g_Arb;
static TaskFactory g_TaskFactory;

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

// Local utility functions
static int PostFile(const char* szUrl, const char* szFilePath, FileInfo* fi); // Depricated
static int PutFile(const char* szUrl, const char* szFilePath, FileInfo* fi); // Depricated
static ret::eCode DeletePost(const std::string& szPostID); // Depricated

// inward faceing functions
int InitializeFileManager();
int InitializeCredentialsManager();
int InitializeEntityManager();

int ShutdownFileManager();
int ShutdownCredentialsManager();
int ShutdownAppInstance();
int ShutdownEntityManager();

int SetWorkingDirectory(const char* szDir);
int SetConfigDirectory(const char* szDir);
int SetTempDirectory(const char* szDir);

int LoadEntity();
int SaveEntity();

int LoadPhraseToken();
int SavePhraseToken(PhraseToken& pt);

int LoadMasterKey(); // call with a valid phrase token

void GetPhraseTokenFilepath(std::string& out);
void GetEntityFilepath(std::string& out);

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey);

int TestQuery()
{
   // EntityManager em;
    
    //g_Entity = em.Discover(g_EntityUrl);
    //em.Discover("https://manuel.tent.is/profile");

}

//////// API start
int InitLibAttic( const char* szWorkingDirectory, 
                  const char* szConfigDirectory,
                  const char* szTempDirectory,
                  const char* szEntityURL,
                  unsigned int threadCount)
{
    // Init sequence ORDER MATTERS
    utils::SeedRand();
    SetConfigDirectory(szConfigDirectory);
    SetWorkingDirectory(szWorkingDirectory);
    SetTempDirectory(szTempDirectory);

    // Essential
    int status = SetEntityUrl(szEntityURL);
    if(status != ret::A_OK)
    {
        std::cout<<"seu FAILED : " << status << std::endl;
    }

    status = InitializeFileManager();
    if(status != ret::A_OK)
    {
            std::cout<<"fm FAILED : " << status << std::endl;
    }

    status = InitializeCredentialsManager();
    if(status != ret::A_OK)
    {
            std::cout<<"cm FAILED : " << status << std::endl;
    }

    status = InitializeEntityManager();
    if(status != ret::A_OK)
    {
        std::cout<<"em FAILED : " << status << std::endl;
    }

    // Non-essential
    LoadAppFromFile();
    LoadAccessToken();

    
    status = g_Arb.Initialize(threadCount);
    if(status != ret::A_OK)
    {
        std::cout<<"arb FAILED : " << status << std::endl;
    }

    status = g_TaskFactory.Initialize();
    if(status != ret::A_OK)
    {
        std::cout<<"Task Factory FAILED : " << status << std::endl;
    }

    std::cout<<"initialization success"<<std::endl;

    // Load Entity Authentication  - ORDER MATTERS
    LoadEntity();
    LoadPhraseToken();

    return status;
}

int ShutdownLibAttic()
{
    int status = ShutdownFileManager();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown filemanger" << std::endl;
    }

    status = ShutdownCredentialsManager();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown credentials manager" << std::endl;
    }
    
    status = ShutdownAppInstance();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown app instance" << std::endl;
    }

    status = ShutdownEntityManager();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown entity manager" << std::endl;
    }

    status = g_Arb.Shutdown();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown task arbiter " << std::endl;

    }

    status = g_TaskFactory.Shutdown();
    if(status != ret::A_OK)
    {
        std::cout<<"FAILED : " << status << " failed to shutdown task factory " << std::endl;
    }

    return status;
}

int InitializeFileManager()
{
    // Construct path
    std::string szFilePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szFilePath);
    szFilePath.append(cnst::g_szManifestName);

    if(!g_pFileManager)
    {
        g_pFileManager = new FileManager(szFilePath, g_ConfigDirectory);
        g_pFileManager->SetTempDirectory(g_TempDirectory);

        if(!g_pFileManager->StartupFileManager())
            return ret::A_FAIL_TO_LOAD_FILE;
    }

    return ret::A_OK;
}

int InitializeCredentialsManager()
{
    int status = ret::A_OK;
    if(!g_pCredManager)
    {
        g_pCredManager = new CredentialsManager();
        g_pCredManager->SetConfigDirectory(g_ConfigDirectory);
        status = g_pCredManager->Initialize();
    }

    return status;
}

int InitializeEntityManager()
{
    int status = ret::A_OK;
    if(!g_pEntityManager)
    {
        g_pEntityManager = new EntityManager();
        status = g_pEntityManager->Initialize();
    }

    return status;
}

int ShutdownFileManager()
{
    // Blind shutdown
    if(g_pFileManager)
    {
        g_pFileManager->ShutdownFileManager();
        delete g_pFileManager;
        g_pFileManager = NULL;
    }

    return ret::A_OK;
}

int ShutdownCredentialsManager()
{
    int status = ret::A_OK;
    if(g_pCredManager)
    {
        while(g_pCredManager->TryLock()) { sleep(0); }
        status = g_pCredManager->Shutdown();
        g_pCredManager->Unlock();

        delete g_pCredManager;
        g_pCredManager = NULL;
    }

    return status;
}

int ShutdownEntityManager()
{
    int status = ret::A_OK;
    if(g_pEntityManager)
    {
        while(g_pEntityManager->TryLock()) { sleep(0); }
        status = g_pEntityManager->Shutdown();
        g_pEntityManager->Unlock();

        delete g_pEntityManager;
        g_pEntityManager = NULL;
    }

    return status;
}

int ShutdownAppInstance()
{
    if(g_pApp)
    {
        delete g_pApp;
        g_pApp = NULL;
    }
    else
    {
        return ret::A_FAIL_INVALID_PTR;
    }

    ConnectionManager::GetInstance()->Shutdown();

    return ret::A_OK;
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
    g_pApp = new TentApp();                                                

    if(szAppName)
        g_pApp->SetAppName(std::string(szAppName));                    
    if(szAppDescription)
        g_pApp->SetAppDescription(std::string(szAppDescription));   
    if(szIcon)
        g_pApp->SetAppIcon(std::string(szIcon));
    if(szUrl)
        g_pApp->SetAppURL(std::string(szUrl));        

    if(redirectUris)
    {
        for(unsigned int i=0; i < uriCount; i++)
        {
            g_pApp->SetRedirectURI(std::string(redirectUris[i]));
        }
    }

    if(scopes)
    {
        for(unsigned int i=0;i<scopeCount; i++)
        {
            g_pApp->SetScope(std::string(scopes[i]));
        }
    }
    
    return ret::A_OK;
}

int RegisterApp(const char* szPostPath)
{
    if(!szPostPath)
        return ret::A_FAIL_INVALID_PTR;

    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string path(szPostPath);
    ConnectionManager* pCm = ConnectionManager::GetInstance();

    // serialize app;
    std::string serialized;
    if(!JsonSerializer::SerializeObject(g_pApp, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    std::cout << " JSON : " << serialized << std::endl;
    
    Response response;

    pCm->HttpPost( path, 
                   NULL,
                   serialized,
                   response);

    std::cout<< " CODE : " << response.code << std::endl;
    std::cout<< " RESPONSE " << response.body << std::endl;

    // Deserialize new data into app
    if(!JsonSerializer::DeserializeObject(g_pApp, response.body))
        return ret::A_FAIL_TO_DESERIALIZE_OBJECT;

    return ret::A_OK;
}

int RequestAppAuthorizationURL(const char* szApiRoot)
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    UrlParams val;
    val.AddValue(std::string("client_id"), g_pApp->GetAppID());

    if(g_pApp->GetRedirectURIs())
    {
        TentApp::RedirectVec* pUris = g_pApp->GetRedirectURIs();
        TentApp::RedirectVec::iterator itr = pUris->begin();

        for(;itr!=pUris->end();itr++)
        {
            val.AddValue(std::string("redirect_uri"), *itr);
        }
    }

    if(g_pApp->GetScopes())
    {
        TentApp::ScopeVec* pScopes = g_pApp->GetScopes();
        TentApp::ScopeVec::iterator itr = pScopes->begin();

        for(;itr!=pScopes->end();itr++)
        {
            val.AddValue(std::string("scope"), *itr);
        }
    }

    val.AddValue("tent_profile_info_types", "all");
    val.AddValue("tent_post_types", "all");
    //val.AddValue("tent_post_types", "https://tent.io/types/posts/status/v0.1.0");

    g_AuthorizationURL.clear();
    g_AuthorizationURL.append(szApiRoot);

    utils::CheckUrlAndAppendTrailingSlash(g_AuthorizationURL);

    g_AuthorizationURL.append("oauth/authorize");

    std::string params;
    val.SerializeToString(params);

    std::cout<<"PARAMS : " << params << std::endl;

    // TODO:: encode these parameters

    g_AuthorizationURL.append(params);

    return ret::A_OK;
}

const char* GetAuthorizationURL()
{
    return g_AuthorizationURL.c_str();
}


int RequestUserAuthorizationDetails(const char* szApiRoot, const char* szCode)
{
    if(!szCode)
        return ret::A_FAIL_INVALID_CSTR;

    // Build redirect code
    RedirectCode rcode;
    rcode.SetCode(std::string(szCode));
    rcode.SetTokenType(std::string("mac"));

    std::string path(szApiRoot);
    utils::CheckUrlAndAppendTrailingSlash(path);
    path.append("apps/");
    path.append(g_pApp->GetAppID());
    path.append("/authorizations");

    std::cout<< " PATH : " << path << std::endl;
    // serialize RedirectCode
    std::string serialized;
    if(!JsonSerializer::SerializeObject(&rcode, serialized))
        return ret::A_FAIL_TO_SERIALIZE_OBJECT;

    Response response;
    ConnectionManager* pCm = ConnectionManager::GetInstance();
    pCm->HttpPostWithAuth( path, 
                           NULL,
                           serialized, 
                           response, 
                           g_pApp->GetMacAlgorithm(), 
                           g_pApp->GetMacKeyID(), 
                           g_pApp->GetMacKey(), 
                           false);

    std::cout<< " CODE : " << response.code << std::endl;
    std::cout<< " RESPONSE : " << response.body << std::endl;

    while(g_pCredManager->TryLock()) { sleep(0); }
    // deserialize auth token
    g_pCredManager->DeserializeIntoAccessToken(response.body);
    g_pCredManager->WriteOutAccessToken();
    g_pCredManager->Unlock();

    return ret::A_OK;
}

int LoadAccessToken()
{
    while(g_pCredManager->TryLock()) { sleep(0); }
    int status = g_pCredManager->LoadAccessToken();
    g_pCredManager->Unlock();

    if(status == ret::A_OK)
    {
        AccessToken at;

        while(g_pCredManager->TryLock()) { sleep(0); }
        g_pCredManager->GetAccessTokenCopy(at);
        g_pCredManager->Unlock();
    }
    
    return status; 
}

int SaveAppToFile()
{
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    return g_pApp->SaveToFile(szSavePath);
}

int LoadAppFromFile()
{
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    // Construct path
    std::string szSavePath(g_ConfigDirectory);
    utils::CheckUrlAndAppendTrailingSlash(szSavePath);
    szSavePath.append(cnst::g_szAppDataName);

    g_pApp->LoadFromFile(szSavePath);

    std::string buffer;
    JsonSerializer::SerializeObject(g_pApp, buffer);
    //std::cout<<" BUFFER : " << buffer << std::endl;

    return ret::A_OK;
}

int PushFile(const char* szFilePath, void (*callback)(int, void*) )
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    while(g_TaskFactory.TryLock()) { sleep(0); };
    Task* t = g_TaskFactory.CreateTentTask( TaskFactory::PUSH,
                                            g_pApp, 
                                            g_pFileManager, 
                                            ConnectionManager::GetInstance(),
                                            g_pCredManager,
                                            at,
                                            g_EntityUrl,
                                            szFilePath,
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);
    g_TaskFactory.Unlock();

    return ret::A_OK;
}

int PullFile(const char* szFilePath, void (*callback)(int, void*))
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    while(g_TaskFactory.TryLock()) { sleep(0); };
    Task* t = g_TaskFactory.CreateTentTask( TaskFactory::PULL,
                                            g_pApp, 
                                            g_pFileManager, 
                                            ConnectionManager::GetInstance(),
                                            g_pCredManager,
                                            at,
                                            g_EntityUrl,
                                            szFilePath,
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);
    g_TaskFactory.Unlock();

    std::cout<<"Returning...."<<std::endl;

    return ret::A_OK;
}

int DeleteFile(const char* szFileName, void (*callback)(int, void*) )
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    while(g_TaskFactory.TryLock()) { sleep(0); };
    Task* t = g_TaskFactory.CreateTentTask( TaskFactory::DELETE,
                                            g_pApp, 
                                            g_pFileManager, 
                                            ConnectionManager::GetInstance(),
                                            g_pCredManager,
                                            at,
                                            g_EntityUrl,
                                            szFileName,
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);
    g_TaskFactory.Unlock();

    return ret::A_OK;
}

int SyncAtticMetaData( void (*callback)(int, void*) )
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    while(g_TaskFactory.TryLock()) { sleep(0); };
    Task* t = g_TaskFactory.CreateTentTask( TaskFactory::SYNCMANIFEST,
                                            g_pApp, 
                                            g_pFileManager, 
                                            ConnectionManager::GetInstance(),
                                            g_pCredManager,
                                            at,
                                            g_EntityUrl,
                                            "",
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);
    g_TaskFactory.Unlock();

    return ret::A_OK;
}

int SyncAtticPostsMetaData(void (*callback)(int, void*))
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    // TODO :: fix this...
    SyncPostsTask* t = new SyncPostsTask( g_pApp, 
                                          g_pFileManager, 
                                          ConnectionManager::GetInstance(),
                                          g_pCredManager,
                                          at,
                                          g_EntityUrl,
                                          "",
                                          g_TempDirectory,
                                          g_WorkingDirectory,
                                          g_ConfigDirectory,
                                          callback);
    g_Arb.SpinOffTask(t);

    return ret::A_OK;
}

int DeleteAllPosts()
{

    return ret::A_OK;
}

int PullAllFiles()
{
    // DEPRICATED
    /*
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;

    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    Manifest::EntriesMap* pEntryMap = g_pFileManager->GetManifestEntries();
    Manifest::EntriesMap::iterator itr = pEntryMap->begin();

    std::string filepath = g_WorkingDirectory;
    utils::CheckUrlAndAppendTrailingSlash(filepath);

    for(;itr != pEntryMap->end(); itr++)
    {
        std::string fn = itr->first;
        PullFile((filepath + fn).c_str(), NULL);
    }
    
    */
    return ret::A_OK;
}

int ChangePassphrase(const char* szOld, const char* szNew)
{
    int status = ret::A_OK;

    status = EnterPassphrase(szOld);

    if(status == ret::A_OK)
    {
        // Get the master key
        MasterKey mk;
        while(g_pCredManager->TryLock()) { sleep(0); }
        g_pCredManager->GetMasterKeyCopy(mk);
        g_pCredManager->Unlock();

        // Register new passphrase with attic
        std::string key;
        mk.GetMasterKey(key);

        status = RegisterPassphraseWithAttic(szNew, key);
    }

    return status;
}

// Master Key
int EnterPassphrase(const char* szPass)
{
    // Enter the passphrase and generate phrase token
    // TODO :: when entering the passphrase always check against the master key with sentinel
    //         if the first 8 bytes don't match (4 bytes == next 4 bytes) then the user entered
    //         the wrong passphrase.
    //         so put a check for that everytime the user enteres the passphrase
    
    // Check for correct passphrase
    // Get Information from entity
    int status = ret::A_OK;

    std::string phrasekey, dirtykey;
    g_Pt.GetPhraseKey(phrasekey);
    g_Pt.GetDirtyKey(dirtykey);
    
    std::string iv, salt;
    g_Pt.GetIv(iv);
    g_Pt.GetSalt(salt);

    std::string phraseKey;
    while(g_pCredManager->TryLock()) { sleep(0); }
    // Enter passphrase to generate key.
    status = g_pCredManager->EnterPassphrase(szPass, salt, phraseKey);
    // Create random master key
    g_pCredManager->Unlock();
    
    if(status == ret::A_OK)
    {
        Credentials enc;
        enc.SetKey(phraseKey);
        enc.SetIv(iv);

        std::string mk;
        // Get encrypted master key that needs decrypting
        Profile* prof = g_Entity.GetFrontProfile();
        if(prof)
        {
            std::cout<<"got prof"<<std::endl;
            AtticProfileInfo* atpi = prof->GetAtticInfo();
            if(atpi)
            {
                atpi->GetMasterKey(mk);
                std::string iiiv;
                atpi->GetIv(iiiv);
                g_Pt.SetIv(iiiv);
            }
        }

        // Attempt to Decrypt Master Key
        std::string out;
        Crypto crypto;
        crypto.DecryptString(mk, enc, out);

        // Check sentinel bytes
        std::string sentone, senttwo;
        sentone = out.substr(0, 4);
        senttwo = out.substr(4, 4);
        
        if(sentone == senttwo)
        {
            // extract actual key apart from sentinel bytes
            std::string keyActual;
            keyActual = out.substr(8);

            // Shove this somewhere
            MasterKey masterKey;
            masterKey.SetMasterKey(keyActual);

            // Insert Into Credentials Manager
            while(g_pCredManager->TryLock()) { sleep(0); }
            g_pCredManager->SetMasterKey(masterKey);
            g_pCredManager->Unlock();

            g_Pt.SetPhraseKey(phraseKey);
            SavePhraseToken(g_Pt);
        }
        else
        {
            status = ret::A_FAIL_SENTINEL_MISMATCH;
        }
    }

    // Return success
    return status;
}

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey)
{
    // Inward facing method
    // Register a new passphrase.
    MasterKey mk;

    while(g_pCredManager->TryLock()) { sleep(0); }
    // Enter passphrase to generate key.
    g_pCredManager->RegisterPassphrase(pass, g_Pt); // This generates a random salt
                                                    // Sets Phrase key
    // Create random master key
    g_pCredManager->CreateMasterKeyWithPass(mk, masterkey); // Create Master Key with given pass
    g_pCredManager->SetMasterKey(mk);
    g_pCredManager->Unlock();

    // Insert sentinel value
    mk.InsertSentinelIntoMasterKey();

    std::string ptsalt;
    g_Pt.GetSalt(ptsalt);  // Phrase Token

    std::string dirtykey;
    //mk.GetMasterKey(key);
    mk.GetMasterKeyWithSentinel(dirtykey);
    g_Pt.SetDirtyKey(dirtykey); // Phrase Token

    // Create Sentinel bytes

    // Setup passphrase cred to encrypt master key
    std::string passphrase;
    g_Pt.GetPhraseKey(passphrase); // Phrase Token

    Crypto crypto;
    // Generate iv
    std::string iv;
    crypto.GenerateIV(iv);

    Credentials enc;
    enc.SetKey(passphrase);
    enc.SetIv(iv);

    g_Pt.SetIv(iv); // Phrase Token

    // Encrypt MasterKey with passphrase key
    std::string out;
    crypto.EncryptString(dirtykey, enc, out);

    std::string salt;
    g_Pt.GetSalt(salt);
    // Create Profile post for 
    AtticProfileInfo* pAtticProf = new AtticProfileInfo();
    // MasterKey with sentinel and Salt
    pAtticProf->SetMasterKey(out);
    pAtticProf->SetSalt(salt);
    pAtticProf->SetIv(iv);

    // Save and post
    std::string output;
    JsonSerializer::SerializeObject(pAtticProf, output);

    std::string url;
    g_Entity.GetFrontProfileUrl(url);

    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    // TODO :: add the type as url params and just pass the attic profile type
    // UrlParams params
    std::string hold("https://cupcake.io/types/info/attic/v0.1.0");
    char *pPm = curl_easy_escape(NULL, hold.c_str() , hold.size());  
    url.append("/");
    url.append(pPm);

    Response resp;
    conops::HttpPost(url, NULL, output, at, resp);

    if(resp.code != 200)
        return ret::A_FAIL_NON_200;


    SavePhraseToken(g_Pt);
    return ret::A_OK;

}

int RegisterPassphrase(const char* szPass, bool override)
{
    // TODO :: probably should check if a passphrase already exists
    // TODO :: probably should include static test case to detect if the passphrase entered was wrong.
    //          - at the begining of the master key append 4 random bytes repeated twice,
    //          - check the first 4 against the latter 4 and if they are the same you entered
    //            the passphrase in correctly.
    //          - obviously skip the first 8 bytes when getting the master key
    //TODO :: figure out way to check if there is a passphrase already set, then warn against overwrite

    std::cout<<" Registering Passphrase ... " << std::endl;

    int status = ret::A_FAIL_REGISTER_PASSPHRASE;

    if(!g_Entity.HasAtticProfile() || override)
    {
        // Register a new passphrase.
        std::string mk;
        while(g_pCredManager->TryLock()) { sleep(0); }
        // Enter passphrase to generate key.
        g_pCredManager->GenerateMasterKey(mk); // Generate random master key
        g_pCredManager->Unlock();

        status = RegisterPassphraseWithAttic(szPass, mk);

    }
    return status;
}


int LoadPhraseToken()
{
    std::cout<< " Loading Phrase Token ... " << std::endl;
    std::string ptpath;
    GetPhraseTokenFilepath(ptpath);
    int status = g_Pt.LoadFromFile(ptpath);
    if(status != ret::A_OK)
    {
        std::cout<<" Loading succeeded ... " << std::endl;
        // Extract Info from entity
        Profile* prof = g_Entity.GetFrontProfile();
        if(prof)
        {
            std::cout<<" profile ... " << std::endl;
            AtticProfileInfo* atpi = prof->GetAtticInfo();
            if(atpi)
            {
                std::cout<<" attic profile " << std::endl;
                std::string salt;
                atpi->GetSalt(salt);
                g_Pt.SetSalt(salt);

                std::string iv;
                atpi->GetIv(iv);
                g_Pt.SetIv(iv);

                std::string key;
                atpi->GetMasterKey(key);
                std::cout<<"Getting master key ... : " << key << std::endl;
                g_Pt.SetDirtyKey(key);

                // Save token to file
                SavePhraseToken(g_Pt);
            }
        }
    }
    else 
    {
        status = ret::A_FAIL_INVALID_PHRASE_TOKEN;
    }

    std::cout<<" Loading phrase token status : " << status << std::endl;
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
    {
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }

    return status;
}

int LoadMasterKey()
{
    int status = ret::A_OK;
    // Check for valid phrase token
   if(!g_Pt.IsPhraseKeyEmpty())
   {
       // "Enter Password"

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

int LoadEntity()
{
    std::string entpath;
    GetEntityFilepath(entpath);
    int status = g_Entity.LoadFromFile(entpath);
 
    if(status != ret::A_OK)
    {
        std::cout<<"discovering...."<<std::endl;
        // Load Entity
        AccessToken at;
        while(g_pCredManager->TryLock()) { sleep(0); }
        g_pCredManager->GetAccessTokenCopy(at);
        g_pCredManager->Unlock();

        // TODO :: Load from file, otherwise discover
        std::cout<< " ACCESS TOKEN : " << at.GetAccessToken() << std::endl;
        status = g_pEntityManager->Discover(g_EntityUrl, at, g_Entity);

        // TODO :: write entity out to file
        g_Entity.WriteToFile(entpath);

    }

    return status;
}

int SaveEntity()
{

    return ret::A_OK;
}

int SaveChanges()
{
    // Use this method to force a system wide save
    
    if(!g_pApp)
        return ret::A_LIB_FAIL_INVALID_APP_INSTANCE;
    
    if(!g_pFileManager)
        return ret::A_LIB_FAIL_INVALID_FILEMANAGER_INSTANCE;

    ret::eCode status = ret::A_OK;

    return status;
}

int SetWorkingDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_WorkingDirectory.append(szDir);

    return ret::A_OK;
}

int SetConfigDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_ConfigDirectory.append(szDir);

    return ret::A_OK;
}

int SetTempDirectory(const char* szDir)
{
    if(!szDir)
        return ret::A_FAIL_INVALID_CSTR;

    g_TempDirectory.append(szDir);

    return ret::A_OK;
}

int SetEntityUrl(const char* szUrl)
{
    if(!szUrl)
        return ret::A_FAIL_INVALID_CSTR;

    g_EntityUrl.append(szUrl);

    return ret::A_OK;
}

const char* GetWorkingDirectory() { return g_WorkingDirectory.c_str(); }
const char* GetConfigDirectory() { return g_ConfigDirectory.c_str(); }
const char* GetEntityUrl() { return g_EntityUrl.c_str(); }

