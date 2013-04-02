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

#include "logutils.h"

static attic::TaskManager*         g_pTaskManager = NULL;      // move to service
static attic::CallbackHandler      g_CallbackHandler;          // move to service
//static TaskArbiter g_Arb;

// var
static std::string g_AuthorizationURL;// move to client
//static PhraseToken  g_Pt;// move to client

static bool g_bEnteredPassphrase = false;
static bool g_bLibInitialized = false;

int LoadMasterKey(); // call with a valid phrase token
int IsLibInitialized(bool checkPassphrase = true);

static attic::Client* g_pClient = NULL;

//////// API start
int InitLibAttic(unsigned int threadCount) {

    int status = attic::ret::A_OK;
    // Init sequence ORDER MATTERS
    attic::utils::SeedRand();

    // Get config values
    std::string workingdir, configdir, tempdir, entityurl;
    attic::ConfigManager::GetInstance()->GetValue(attic::cnst::g_szConfigWorkingDir, workingdir);
    attic::ConfigManager::GetInstance()->GetValue(attic::cnst::g_szConfigConfigDir, configdir);
    attic::ConfigManager::GetInstance()->GetValue(attic::cnst::g_szConfigTempDir, tempdir);
    attic::ConfigManager::GetInstance()->GetValue(attic::cnst::g_szConfigEntityURL, entityurl);

    std::string t;
    std::cout<<" creating new client ... " << std::endl;
    if(!workingdir.empty())
        attic::fs::CreateDirectory(workingdir);
    if(!configdir.empty())
        attic::fs::CreateDirectory(configdir);
    if(!tempdir.empty())
        attic::fs::CreateDirectory(tempdir);
    
    g_pClient = new attic::Client(workingdir,
                           configdir,
                           tempdir,
                           entityurl);
                          
    g_pClient->Initialize();
    g_pClient->LoadAppFromFile();
    g_pClient->LoadAccessToken();
    g_pClient->LoadEntity();
    g_pClient->LoadPhraseToken();

    if(status == attic::ret::A_OK)  {
        attic::event::EventSystem::GetInstance()->Initialize();
        // Essential
        status = attic::liba::InitializeTaskArbiter(threadCount);
        // Try to load a master key if we have one
        if(LoadMasterKey() == attic::ret::A_OK) {  // don't set it equal to status, because if this fails
                                            // it's really not that important, we can have the user
                                            // go ahead and enter it.
        }

        status = attic::liba::InitializeTaskManager(&g_pTaskManager,
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
        status = attic::ret::A_FAIL_LOAD_APP_DATA;
    }

    if(status == attic::ret::A_OK) { 
        g_bLibInitialized = true;
    }

    return status;
}

int ShutdownLibAttic(void (*callback)(int, void*)) {
    int status = attic::ret::A_OK;

    // Shutdown threading first, ALWAYS
    status = attic::liba::ShutdownTaskArbiter();
    status = attic::liba::ShutdownTaskManager(&g_pTaskManager);
    attic::event::ShutdownEventSystem();
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
    int status = attic::app::RegisterAttic(szEntityurl,
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

    int status = attic::ret::A_OK;
    status = attic::app::RequestUserAuthorizationDetails(szEntityUrl,
                                                  szCode, 
                                                  szConfigDirectory);
    return status;
}


int PushFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == attic::ret::A_OK){
        try {
            std::string filepath(szFilePath);
            attic::event::RaiseEvent(attic::event::Event::REQUEST_PUSH, filepath, NULL);
        }
        catch(std::exception& e) {
            attic::log::LogException("TOP#!FKDA", e);
        }
    }
    return status;
}

int PullFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == attic::ret::A_OK){
        try {
            std::string filepath(szFilePath);
            attic::event::RaiseEvent(attic::event::Event::REQUEST_PULL, filepath, NULL);
        }
        catch(std::exception& e) {
            attic::log::LogException("TOP1414", e);
        }
    }

    return attic::ret::A_OK;
}

int DeleteFile(const char* szFilePath) {
    int status = IsLibInitialized();

    if(status == attic::ret::A_OK) {
        try { 
            std::string filepath(szFilePath);
            attic::event::RaiseEvent(attic::event::Event::REQUEST_DELETE, filepath, NULL);
        }
        catch(std::exception& e) {

        }
    }

    return status;
}

int PollFiles(void) {
    int status = IsLibInitialized();

    if(status == attic::ret::A_OK) {
        try { 
            if(status == attic::ret::A_OK)
                status = g_pTaskManager->PollFiles(NULL);
        }
        catch(std::exception& e) {
            attic::log::LogException("TOP12341", e);
        }
    }



    return status;
}

int RegisterPassphrase(const char* szPass, bool override) {
    if(!szPass) return attic::ret::A_FAIL_INVALID_CSTR;
    //int status = IsLibInitialized(false);
    int status = attic::ret::A_OK;
    if(status == attic::ret::A_OK) {
        status = attic::ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        attic::pass::Passphrase ps(g_pClient->entity(), g_pClient->access_token());
        // Generate Master Key
        std::string master_key;
        g_pClient->credentials_manager()->GenerateMasterKey(master_key); // Generate random master key
        std::string recovery_key;
        status = ps.RegisterPassphrase(szPass, master_key, recovery_key, override);

        if(status == attic::ret::A_OK) {
            attic::event::RaiseEvent(attic::event::Event::RECOVERY_KEY, recovery_key, NULL);
        }
    }
    return status;
}

int EnterPassphrase(const char* szPass) {
    if(!szPass) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = IsLibInitialized(false);
    if(status == attic::ret::A_OK) {
        status = attic::ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        attic::pass::Passphrase ps(g_pClient->entity(), g_pClient->access_token());

        std::string master_key;
        attic::PhraseToken pt;
        status = ps.EnterPassphrase(szPass, pt, master_key);

        if(status == attic::ret::A_OK) {
            g_pClient->set_phrase_token(pt);
            g_pClient->credentials_manager()->set_master_key(master_key);
            g_pClient->SavePhraseToken();
            g_bEnteredPassphrase = true;
        }
    }

    return status;
}

int ChangePassphrase(const char* szOld, const char* szNew) {
    if(!szOld) return attic::ret::A_FAIL_INVALID_CSTR;
    if(!szNew) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = IsLibInitialized(false);
    if(status == attic::ret::A_OK) {
        status = attic::ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        attic::pass::Passphrase ps(g_pClient->entity(), g_pClient->access_token());

        std::string recovery_key;
        status = ps.ChangePassphrase(szOld, szNew, recovery_key);
        if(status == attic::ret::A_OK){
            attic::event::RaiseEvent(attic::event::Event::RECOVERY_KEY, recovery_key, NULL);
        }
    }
    return status;
}

int EnterRecoveryKey(const char* szRecovery) {
    if(!szRecovery) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = IsLibInitialized(false);
    if(status == attic::ret::A_OK) {
        status = attic::ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        attic::pass::Passphrase ps(g_pClient->entity(), g_pClient->access_token());

        std::string temp_pass;
        status = ps.EnterRecoveryKey(szRecovery, temp_pass);
        if(status == attic::ret::A_OK){
            attic::event::RaiseEvent(attic::event::Event::TEMPORARY_PASS, temp_pass, NULL);
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
    int status = g_pClient->LoadEntity(true);
    if(status == attic::ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(g_pClient->credentials_manager()) {
            attic::MasterKey mk;
            std::string masterkey;
            g_pClient->credentials_manager()->GetMasterKeyCopy(mk);
            mk.GetMasterKey(masterkey);
            attic::Entity ent = g_pClient->entity();
            status = attic::pass::RegisterRecoveryQuestionsWithAttic(masterkey, 
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
            status = attic::ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
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
    if(status == attic::ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(g_pClient->credentials_manager()) {
            std::string mkOut;
            attic::Entity ent = g_pClient->entity();
            status = attic::pass::EnterRecoveryQuestions(g_pClient->credentials_manager(),
                                                  ent,
                                                  questionOne,
                                                  questionTwo,
                                                  questionThree,
                                                  answerOne,
                                                  answerTwo,
                                                  answerThree,
                                                  mkOut);
            if(status == attic::ret::A_OK) {
                std::string temppass;
                attic::utils::GenerateRandomString(temppass, 16);
                std::string new_recovery_key;
                status = attic::pass::RegisterPassphraseWithAttic(temppass, 
                                                           mkOut,
                                                           g_pClient,
                                                           new_recovery_key);

                if(status == attic::ret::A_OK) {
                    attic::event::RaiseEvent(attic::event::Event::TEMPORARY_PASS, temppass, NULL);
                }
            }
        }
        else {
            status = attic::ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
        }
    }
    return status;
}

int PhraseStatus() {
    int status = attic::ret::A_OK;

    if(!g_bEnteredPassphrase)
        status = attic::ret::A_FAIL_NEED_ENTER_PASSPHRASE;

    return status;
}

int LoadMasterKey() { // Depricated

    std::cout<<" LoadMasterKey in libattic.cpp is depricated, implement functionality in client"<<std::endl;
    int status = attic::ret::A_OK;
    /*
    // Check for valid phrase token
    PhraseToken pt = g_pClient->phrase_token();
    if(pt.IsPhraseKeyEmpty()) {
        // "Enter Password"
        g_bEnteredPassphrase = false;
        status = attic::ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }
    else {
        std::string phraseKey = pt.phrase_key();
        std::string salt = pt.salt();
        status = DecryptMasterKey(phraseKey, salt);
    }   
    */

    return status;
}

int IsLibInitialized(bool checkPassphrase) {
    int status = attic::ret::A_OK;
    if(!g_bLibInitialized) status = attic::ret::A_FAIL_LIB_INIT;
    return status;
}

int GetFileList(void(*callback)(int, char**, int, int)) {
    int status = attic::ret::A_OK;
    if(!callback)
        status = attic::ret::A_FAIL_INVALID_PTR;

    if(status == attic::ret::A_OK) {
        status = IsLibInitialized();

        if(status == attic::ret::A_OK)
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
        g_CallbackHandler.RegisterCallback(attic::event::Event::PULL, callback);
}

void RegisterForPushNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::PUSH, callback);
}

void RegisterForUploadSpeedNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::UPLOAD_SPEED, callback);
}

void RegisterForDownloadSpeedNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::DOWNLOAD_SPEED, callback);
}

void RegisterForErrorNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::ERROR_NOTIFY, callback);
}

void RegisterForRecoveryKeyNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::RECOVERY_KEY, callback);
}

void RegisterForTemporaryKeyNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::TEMPORARY_PASS, callback);
}

void RegisterForPauseResumeNotify(void (*callback)(int, int, const char*)) {
    if(callback)
        g_CallbackHandler.RegisterCallback(attic::event::Event::PAUSE_RESUME_NOTIFY, callback);
}

int Pause(void) { 
    attic::event::RaiseEvent(attic::event::Event::PAUSE, "", NULL);
}

int Resume(void) {
    attic::event::RaiseEvent(attic::event::Event::RESUME, "", NULL);
}

void SetConfigValue(const char* szKey, const char* szValue) {
    std::string key(szKey);
    std::string value(szValue);
    attic::ConfigManager::GetInstance()->SetValue(key, value);
}

