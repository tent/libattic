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

#include "constants.h"
#include "credentials.h"
#include "profile.h"
#include "conoperations.h"

#include "libatticutils.h"

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

// inward faceing functions
int SetWorkingDirectory(const char* szDir);
int SetConfigDirectory(const char* szDir);
int SetTempDirectory(const char* szDir);

int LoadEntity(bool override = false);
int SaveEntity();

int LoadPhraseToken();
int SavePhraseToken(PhraseToken& pt);
int LoadMasterKey(); // call with a valid phrase token
int SetFileManagerMasterKey();

void GetPhraseTokenFilepath(std::string& out);
void GetEntityFilepath(std::string& out);

int RegisterPassphraseWithAttic(const std::string& pass, const std::string& masterkey);

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv);

//TODO TESTING METHODS REMOVE
FileManager* GetFileManager() { return g_pFileManager; }

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
        std::cout<<"seu FAILED : " << status << std::endl;

    status = liba::InitializeFileManager( &g_pFileManager,
                                          cnst::g_szManifestName,
                                          g_ConfigDirectory,
                                          g_TempDirectory );
    if(status != ret::A_OK)
        std::cout<<"fm FAILED : " << status << std::endl;

    status = liba::InitializeCredentialsManager( &g_pCredManager,
                                                 g_ConfigDirectory);
    if(!g_pCredManager)
        std::cout<<"failed to create cred manager " << std::endl;

    if(status != ret::A_OK)
        std::cout<<"cm FAILED : " << status << std::endl;

    status = liba::InitializeEntityManager( &g_pEntityManager );
    if(status != ret::A_OK)
        std::cout<<"em FAILED : " << status << std::endl;

    // Non-essential
    LoadAppFromFile();
    LoadAccessToken();
    
    status = g_Arb.Initialize(threadCount);
    if(status != ret::A_OK)
        std::cout<<"arb FAILED : " << status << std::endl;

    status = g_TaskFactory.Initialize();
    if(status != ret::A_OK)
        std::cout<<"Task Factory FAILED : " << status << std::endl;

    // Load Entity Authentication  - ORDER MATTERS
    LoadEntity();
    
    status = LoadPhraseToken();
    if(status != ret::A_OK)
        std::cout<<"Load Phrase FAILED : " << status << std::endl;

    status = LoadMasterKey();
    if(status != ret::A_OK)
        std::cout<<"Load Master Key FAILED : " << status << std::endl;
    else
        status = SetFileManagerMasterKey();

    
    std::cout<<"initialization success"<<std::endl;
    return status;
}

int SetFileManagerMasterKey()
{
    // If loaded Set master key in filemanager
    MasterKey mk;
    g_pCredManager->Lock();
    g_pCredManager->GetMasterKeyCopy(mk);
    g_pCredManager->Unlock();

    g_pFileManager->Lock();
    g_pFileManager->SetMasterKey(mk);
    g_pFileManager->Unlock();
    
    return ret::A_OK;
}

int ShutdownLibAttic()
{
    int status = ret::A_OK;

    // Shutdown threading first, ALWAYS
    status = g_Arb.Shutdown();
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown task arbiter " << std::endl;

    status = g_TaskFactory.Shutdown();
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown task factory " << std::endl;

    status = liba::ShutdownFileManager(g_pFileManager);
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown filemanger" << std::endl;

    status = liba::ShutdownCredentialsManager(g_pCredManager);
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown credentials manager" << std::endl;
    
    status = liba::ShutdownAppInstance(g_pApp);
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown app instance" << std::endl;

    status = liba::ShutdownEntityManager(g_pEntityManager);
    if(status != ret::A_OK)
        std::cout<<"FAILED : " << status << " failed to shutdown entity manager" << std::endl;

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
        return ret::A_FAIL_INVALID_APP_INSTANCE;

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
        return ret::A_FAIL_INVALID_APP_INSTANCE;

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
    g_pCredManager->Lock();
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
        return ret::A_FAIL_INVALID_APP_INSTANCE;

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
    g_pCredManager->Lock();
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    Task* t = g_TaskFactory.SynchronousGetTentTask( TaskFactory::PUSH,
                                            g_pApp, 
                                            g_pFileManager, 
                                            g_pCredManager,
                                            &g_Arb,
                                            &g_TaskFactory,
                                            at,
                                            g_EntityUrl,
                                            szFilePath,
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);

    return ret::A_OK;
}

int PullFile(const char* szFilePath, void (*callback)(int, void*))
{
    AccessToken at;
    g_pCredManager->Lock();
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    Task* t = g_TaskFactory.SynchronousGetTentTask( TaskFactory::PULL,
                                            g_pApp, 
                                            g_pFileManager, 
                                            g_pCredManager,
                                            &g_Arb,
                                            &g_TaskFactory,
                                            at,
                                            g_EntityUrl,
                                            szFilePath,
                                            g_TempDirectory,
                                            g_WorkingDirectory,
                                            g_ConfigDirectory,
                                            callback);
    g_Arb.SpinOffTask(t);

    std::cout<<"Returning...."<<std::endl;

    return ret::A_OK;
}

int PullAllFiles(void (*callback)(int, void*))
{
    AccessToken at;
    g_pCredManager->Lock();
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    Task* t = g_TaskFactory.SynchronousGetTentTask( TaskFactory::PULLALL,
                                             g_pApp, 
                                             g_pFileManager, 
                                             g_pCredManager,
                                             &g_Arb,
                                             &g_TaskFactory,
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

int SyncFiles(void (*callback)(int, void*))
{
    // TODO :: this

    return ret::A_OK;
}

int DeleteFile(const char* szFileName, void (*callback)(int, void*) )
{
    AccessToken at;
    g_pCredManager->Lock();
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    Task* t = g_TaskFactory.SynchronousGetTentTask( TaskFactory::DELETE,
                                             g_pApp, 
                                             g_pFileManager, 
                                             g_pCredManager,
                                             &g_Arb,
                                             &g_TaskFactory,
                                             at,
                                             g_EntityUrl,
                                             szFileName,
                                             g_TempDirectory,
                                             g_WorkingDirectory,
                                             g_ConfigDirectory,
                                             callback);
    g_Arb.SpinOffTask(t);

    return ret::A_OK;
}

int DeleteAllPosts()
{
    AccessToken at;
    while(g_pCredManager->TryLock()) { sleep(0); }
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    // Completely nuke all posts associated with the account
    std::string url("https://manuel.tent.is/tent/posts");

                                                      //const UrlParams* pParams,
    Response response;
    UrlParams params;
    params.AddValue("type", cnst::g_szAtticPostType);
    ConnectionManager::GetInstance()->HttpGetWithAuth( url,
                                                     &params,
                                                     response,
                                                     at.GetMacAlgorithm(),
                                                     at.GetAccessToken(),
                                                     at.GetMacKey(),
                                                     true);

    std::cout<<" RESPONSE : " << response.code << std::endl;
    std::cout<<" BODY : " << response.body << std::endl;
   

    return ret::A_OK;
}



int ChangePassphrase(const char* szOld, const char* szNew)
{
    // TODO :: this should be a task
    //         pull all files
    //         decrypt them with the old key
    //         re-encrypt them with the new key
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


int GetMasterKeyFromProfile(std::string& out)
{
    int status = ret::A_OK;

    Profile* prof = g_Entity.GetFrontProfile();
    if(prof)
    {
        AtticProfileInfo* atpi = prof->GetAtticInfo();
        if(atpi)
        {
            atpi->GetMasterKey(out);
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_PTR;
    }

    return status;
}

int DecryptMasterKey(const std::string& phraseKey, const std::string& iv)
{
    int status = ret::A_OK;

    if(!phraseKey.empty())
    {
        Credentials cred;
        cred.SetKey(phraseKey);
        cred.SetIv(iv);

        std::string encmk; // Encrypted Master Key
        status = GetMasterKeyFromProfile(encmk);
        if(status == ret::A_OK)
        {
            if(!encmk.empty())
            {
                // Attempt to Decrypt Master Key
                std::string out;
                Crypto crypto;
                status = crypto.DecryptStringCFB(encmk, cred, out);

                if(status == ret::A_OK)
                {
                    if(out.size() >= 8)
                    {
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
                            g_pCredManager->Lock();
                            g_pCredManager->SetMasterKey(masterKey);
                            g_pCredManager->Unlock();

                            g_Pt.SetPhraseKey(phraseKey);
                            SavePhraseToken(g_Pt);
                            g_bEnteredPassphrase = true;
                        }
                        else
                        {
                            status = ret::A_FAIL_SENTINEL_MISMATCH;
                        }
                    }
                    else
                    {
                        status = ret::A_FAIL_OTHER;
                    }
                }

            }
            else
            {
                status = ret::A_FAIL_INVALID_MASTERKEY;
            }
        }
    }
    else
    {
        status = ret::A_FAIL_EMPTY_PASSPHRASE;
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

    // Force download entity
    LoadEntity(true);

    std::string salt;
    g_Pt.GetSalt(salt);

    std::string phraseKey;

    g_pCredManager->Lock();
    status = g_pCredManager->EnterPassphrase(szPass, salt, phraseKey); // Enter passphrase to generate key.
    g_pCredManager->Unlock();

    if(status == ret::A_OK)
    {
        std::cout<< " PASS : " << szPass << std::endl;
        std::cout<< " PHRASE KEY : " << phraseKey << std::endl;
        std::cout<< " salt : " << salt << std::endl;

        status = DecryptMasterKey(phraseKey, salt);
        if(status == ret::A_OK)
        {
            // Reload phrase token
            //LoadPhraseToken();
            // Load Master Key
            status = LoadMasterKey();
            if(status == ret::A_OK)
                status = SetFileManagerMasterKey();
        }
        std::cout<<" DECRYPT STATUS : " << status << std::endl;

    }

    return status;
}

int ConstructMasterKey(const std::string& masterkey, MasterKey& out)
{
    int status = ret::A_OK;

    g_pCredManager->Lock();
    // Enter passphrase to generate key.
    g_pCredManager->RegisterPassphrase(masterkey, g_Pt); // This generates a random salt
                                                         // Sets Phrase key
    g_pCredManager->CreateMasterKeyWithPass(out, masterkey); // Create Master Key with given pass
    g_pCredManager->SetMasterKey(out);
    g_pCredManager->Unlock();

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

    std::cout<< "Encrypting key with passphrase key : " << phrasekey << std::endl;
    std::cout<< "Using iv : " << salt << std::endl;
    // Encrypt MasterKey with passphrase key
    std::string out;
    crypto.EncryptStringCFB(key, enc, keyOut);
    
    return status;
} 

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
    JsonSerializer::SerializeObject(pAtticProf, output);

    std::string url;
    g_Entity.GetFrontProfileUrl(url);

    AccessToken at;
    g_pCredManager->Lock();
    g_pCredManager->GetAccessTokenCopy(at);
    g_pCredManager->Unlock();

    // TODO :: add the type as url params and just pass the attic profile type
    // UrlParams params
    std::string hold(cnst::g_szAtticProfileType);
    char *pPm = curl_easy_escape(NULL, hold.c_str() , hold.size());  
    url.append("/");
    url.append(pPm);

    Response resp;
    conops::HttpPut(url, NULL, output, at, resp);

    std::cout<<" URL : " << url << std::endl;
    std::cout<<" REGISTER RESP CODE : " << resp.code << std::endl;
    std::cout<<" BODY : " << resp.body << std::endl;

    if(resp.code != 200)
        return ret::A_FAIL_NON_200;

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
    std::cout<<" GENERATED MASTER KEY : " << genmk << std::endl;

    if(status == ret::A_OK)
    {
        // Insert sentinel value
        mk.InsertSentinelIntoMasterKey();
        
        // Get Salt
        std::string salt;
        g_Pt.GetSalt(salt);  // Phrase Token


        // Get Dirty Master Key (un-encrypted master key with sentinel values)
        std::string dirtykey;
        mk.GetMasterKeyWithSentinel(dirtykey);
        g_Pt.SetDirtyKey(dirtykey); // Set Phrase Token

        std::cout<<" MASTER KEY WITH SENTENEL : " << dirtykey << std::endl;
        
        // Enter passphrase to generate key.

        std::string phraseKey;
        status = g_pCredManager->EnterPassphrase(pass, salt, phraseKey);
        if(status == ret::A_OK)
        {
            std::cout<< " PHRASE KEY : " << phraseKey << std::endl;
            std::cout<<" Salt : " << salt << std::endl;
            g_Pt.SetPhraseKey(phraseKey);

            // Setup passphrase cred to encrypt master key
            std::string encryptedkey;
            status = EncryptKeyWithPassphrase( dirtykey, 
                                               phraseKey, 
                                               salt, 
                                               encryptedkey);

            if(status == ret::A_OK)
            {
                std::cout<<" ENCRYPTED KEY : " << encryptedkey << std::endl;
                status = RegisterPassphraseProfilePost( encryptedkey, 
                                                        salt);
                if(status == ret::A_OK)
                    SavePhraseToken(g_Pt);
            }
        }
    }
    else
    {
        status = ret::A_FAIL_INVALID_MASTERKEY;
    }
       
    return status; 
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

    if(!g_Entity.HasAtticProfileMasterKey() || override)
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
    std::string ptpath;
    GetPhraseTokenFilepath(ptpath);

    int status = g_Pt.LoadFromFile(ptpath);
    if(status != ret::A_OK)
    {
        // couldn't load from file (non existant)
        // Extract partial info
        // Extract Info from entity
        Profile* prof = g_Entity.GetFrontProfile();
        if(prof)
        {
            AtticProfileInfo* atpi = prof->GetAtticInfo();
            if(atpi)
            {
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
        else
        {
            status = ret::A_FAIL_INVALID_PTR;
        }
    }
    else 
    {
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
    {
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }

    return status;
}

int LoadMasterKey()
{
    std::cout<<" Loading master key ... " << std::endl;
    int status = ret::A_OK;
    // Check for valid phrase token
    if(g_Pt.IsPhraseKeyEmpty())
    {
        // "Enter Password"
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }
    else
    {
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
 
    if(status != ret::A_OK || override)
    {
        if(override)
            g_Entity.ResetEntity();

        // Load Entity
        AccessToken at;
        g_pCredManager->Lock();
        g_pCredManager->GetAccessTokenCopy(at);
        g_pCredManager->Unlock();

        status = g_pEntityManager->Discover(g_EntityUrl, at, g_Entity);

        if(status == ret::A_OK)
            g_Entity.WriteToFile(entpath);
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

