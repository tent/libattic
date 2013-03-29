#include "libattic.h"
#include "callbackhandler.h"

#include <fstream>
#include <string>
#include <sstream>

#include "utils.h"
#include "errorcodes.h"
#include "apputils.h"
#include "tentapp.h"

#include "taskfactory.h"
#include "taskarbiter.h"

#include "constants.h"
#include "credentials.h"
#include "profile.h"

#include "libatticutils.h"

#include "taskmanager.h"

#include "event.h"
#include "callbackhandler.h"
#include "passphrase.h"

#include "atticclient.h"
#include "clientutils.h"
#include "configmanager.h"

static TaskManager*         g_pTaskManager = NULL;      // move to service
static CallbackHandler      g_CallbackHandler;          // move to service
//static TaskArbiter g_Arb;

// var
static std::string g_AuthorizationURL;// move to client
//static PhraseToken  g_Pt;// move to client

static bool g_bEnteredPassphrase = false;
static bool g_bLibInitialized = false;

int LoadMasterKey(); // call with a valid phrase token
int DecryptMasterKey(const std::string& phraseKey, const std::string& iv);
int IsLibInitialized(bool checkPassphrase = true);

static Client* g_pClient = NULL;

//////// API start
int InitLibAttic(unsigned int threadCount) {

    int status = ret::A_OK;
    // Init sequence ORDER MATTERS
    utils::SeedRand();

    // Get config values
    std::string workingdir, configdir, tempdir, entityurl;
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigWorkingDir, workingdir);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigConfigDir, configdir);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigTempDir, tempdir);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigEntityURL, entityurl);

    std::string t;
    std::cout<<" creating new client ... " << std::endl;
    if(!workingdir.empty())
        fs::CreateDirectory(workingdir);
    if(!configdir.empty())
        fs::CreateDirectory(configdir);
    if(!tempdir.empty())
        fs::CreateDirectory(tempdir);
    
    g_pClient = new Client(workingdir,
                           configdir,
                           tempdir,
                           entityurl);
                          
    g_pClient->Initialize();
    g_pClient->LoadAppFromFile();
    g_pClient->LoadAccessToken();
    g_pClient->LoadEntity();
    g_pClient->LoadPhraseToken();

    if(status == ret::A_OK)  {
        event::EventSystem::GetInstance()->Initialize();
        // Essential
        status = liba::InitializeTaskArbiter(threadCount);
        // Try to load a master key if we have one
        if(LoadMasterKey() == ret::A_OK) {  // don't set it equal to status, because if this fails
                                            // it's really not that important, we can have the user
                                            // go ahead and enter it.
        }

        status = liba::InitializeTaskManager(&g_pTaskManager,
                                             g_pClient->file_manager(),
                                             g_pClient->credentials_manager(),
                                             g_pClient->access_token(),
                                             g_pClient->entity(),
                                             g_pClient->temp_directory(),
                                             g_pClient->working_directory(),
                                             g_pClient->config_directory());

        g_CallbackHandler.Initialize();
    }
    else {
        status = ret::A_FAIL_LOAD_APP_DATA;
    }

    if(status == ret::A_OK) { 
        g_bLibInitialized = true;
    }

    return status;
}

int ShutdownLibAttic(void (*callback)(int, void*)) {
    int status = ret::A_OK;

    // Shutdown threading first, ALWAYS
    status = liba::ShutdownTaskArbiter();
    status = liba::ShutdownTaskManager(&g_pTaskManager);
    event::ShutdownEventSystem();
    g_pTaskManager = NULL;

    if(g_pClient) {
        std::cout<<" shutting down client ... " << std::endl;
        g_pClient->Shutdown();

        std::cout<<" cleaning up client ! ... " << std::endl;
        delete g_pClient;
        g_pClient = NULL;
    }

    std::cout<<" calling back .. " << std::endl;
    if(callback)
        callback(status, NULL);

    g_bLibInitialized = false;

    std::cout<<" done  " << std::endl;
    return status;
}

int RegisterAtticApp(const char* szEntityurl,
                     const char* szAppName, 
                     const char* szAppDescription, 
                     const char* szUrl, 
                     const char* szIcon, 
                     const char* szRedirectUri, 
                     const char* szConfigDir) {
    int status = app::RegisterAttic(szEntityurl,
                                szAppName,
                                szAppDescription,
                                szUrl,
                                szIcon,
                                szRedirectUri,
                                szConfigDir,
                                g_AuthorizationURL);
    return status;
}

const char* GetAuthorizationURL() {
    return g_AuthorizationURL.c_str();
}

int RequestUserAuthorizationDetails(const char* szEntityUrl,
                                    const char* szCode, 
                                    const char* szConfigDirectory) {

    int status = ret::A_OK;
    status = app::RequestUserAuthorizationDetails(szEntityUrl,
                                                  szCode, 
                                                  szConfigDirectory);
    return status;
}


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

int PollFiles(void) {
    int status = IsLibInitialized();

    if(status == ret::A_OK)
        status = g_pTaskManager->PollFiles(NULL);

    return status;
}

int GetMasterKeyFromProfile(std::string& out) {
    
    std::cout<<" Getting master key from profile ... " << std::endl;
    int status = ret::A_OK;
/*
    Profile* prof = g_pClient->entity()->GetFrontProfile();
    if(prof) {
        AtticProfileInfo* atpi = prof->GetAtticInfo();
        if(atpi)
            atpi->GetMasterKey(out);
    }
    else
        status = ret::A_FAIL_INVALID_PROFILE;
        */

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
                            g_pClient->credentials_manager()->SetMasterKey(masterKey);
                            // Set the phrase key
                            g_pClient->set_phrase_key(phraseKey);
                            g_bEnteredPassphrase = true;
                        }
                        else {
                            status = ret::A_FAIL_SENTINEL_MISMATCH;
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
    if(!szPass) return ret::A_FAIL_INVALID_CSTR;
    int status = IsLibInitialized(false);
    if(status == ret::A_OK) {
        status = ret::A_FAIL_REGISTER_PASSPHRASE;

        // Discover Entity, get access token
        pass::Passphrase ps(g_pClient->entity(), g_pClient->access_token());

        // Generate Master Key
        std::string master_key;
        g_pClient->credentials_manager()->GenerateMasterKey(master_key); // Generate random master key
        std::string recovery_key;
        status = ps.RegisterPassphrase(szPass, master_key, recovery_key);

        if(status == ret::A_OK) {
            event::RaiseEvent(event::Event::RECOVERY_KEY, recovery_key, NULL);
            // call enter password?
        }
    }

    return status;
}

int EnterPassphrase(const char* szPass) {
    int status = IsLibInitialized(false);

   /* 
    if(status == ret::A_OK) {
        // Enter the passphrase and generate phrase token
        // TODO :: when entering the passphrase always check against the master key with sentinel
        //         if the first 8 bytes don't match (4 bytes == next 4 bytes) then the user entered
        //         the wrong passphrase.
        //         so put a check for that everytime the user enteres the passphrase
        
        // Check for correct passphrase
        // Get Information from entity

        // Force download entity
        g_pClient->LoadEntity(true);

        std::string salt;
        g_pClient->phrase_token()->salt(salt);

        std::cout<<" entering pass ... " << std::endl;
        std::string phraseKey;
        status = g_pClient->credentials_manager()->EnterPassphrase(szPass, salt, phraseKey); // Enter passphrase to generate key.

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
*/
    return status;
}

int ChangePassphrase(const char* szOld, const char* szNew) {
    int status = IsLibInitialized(false);
/*
    if(status == ret::A_OK) {
        status = EnterPassphrase(szOld);

        if(status == ret::A_OK) {
            // Get the master key
            MasterKey mk;
            g_pClient->credentials_manager()->GetMasterKeyCopy(mk);

            // Register new passphrase with attic
            std::string key;
            mk.GetMasterKey(key);

            std::string recoverykey;
            status = pass::RegisterPassphraseWithAttic( std::string(szNew), 
                                                        key, // master key
                                                        g_pClient,
                                                        recoverykey);
            if(status == ret::A_OK){
                event::RaiseEvent(event::Event::RECOVERY_KEY, recoverykey, NULL);
            }
        }
    }
    */

    return status;
}

int EnterRecoveryKey(const char* szRecovery) {

    int status = IsLibInitialized(false);
/*

    if(!szRecovery) status = ret::A_FAIL_INVALID_CSTR;
    if(status == ret::A_OK) {

        std::string recovery_key(szRecovery);
        status = g_pClient->LoadEntity(true);
        if(status == ret::A_OK) {
            std::string masterkey;
            status = pass::EnterRecoveryKey(recovery_key, 
                                            g_pClient->credentials_manager(), 
                                            *(g_pClient->entity()),
                                            masterkey);

            std::cout<<" MASTER KEY : " << masterkey << std::endl;
            if(status == ret::A_OK) {
                std::string temppass;
                utils::GenerateRandomString(temppass, 16);
                std::string new_recovery_key;
                status = pass::RegisterPassphraseWithAttic(temppass, 
                                                           masterkey,
                                                           g_pClient,
                                                           new_recovery_key);

                if(status == ret::A_OK) {
                    //status = EnterPassphrase(temppass); // Load phrase token, and write out to ent file
                    event::RaiseEvent(event::Event::TEMPORARY_PASS, temppass, NULL);
                }
            }
        }
    }
*/
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
    int status = g_pClient->LoadEntity(true);
    if(status == ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(g_pClient->credentials_manager()) {
            MasterKey mk;
            std::string masterkey;
            g_pClient->credentials_manager()->GetMasterKeyCopy(mk);
            mk.GetMasterKey(masterkey);
            Entity ent = g_pClient->entity();
            status = pass::RegisterRecoveryQuestionsWithAttic(masterkey, 
                                                              g_pClient->credentials_manager(),
                                                              ent, 
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
    int status = g_pClient->LoadEntity(true);
    if(status == ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(g_pClient->credentials_manager()) {
            std::string mkOut;
            Entity ent = g_pClient->entity();
            status = pass::EnterRecoveryQuestions(g_pClient->credentials_manager(),
                                                  ent,
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
                                                           g_pClient,
                                                           new_recovery_key);

                if(status == ret::A_OK) {
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

int PhraseStatus() {
    int status = ret::A_OK;

    if(!g_bEnteredPassphrase)
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;

    return status;
}

int LoadMasterKey() {
    int status = ret::A_OK;
    // Check for valid phrase token
    PhraseToken pt = g_pClient->phrase_token();
    if(pt.IsPhraseKeyEmpty()) {
        // "Enter Password"
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }
    else {
        std::string phraseKey = pt.phrase_key();
        std::string salt = pt.salt();
        status = DecryptMasterKey(phraseKey, salt);
    }   

    return status;
}

int IsLibInitialized(bool checkPassphrase) {
    int status = ret::A_OK;
    if(!g_bLibInitialized) status = ret::A_FAIL_LIB_INIT;
    return status;
}

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

void SetConfigValue(const char* szKey, const char* szValue) {
    std::string key(szKey);
    std::string value(szValue);
    ConfigManager::GetInstance()->SetValue(key, value);
}

