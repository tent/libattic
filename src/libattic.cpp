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

    std::string t;
    std::cout<<" creating new client ... " << std::endl;
    g_pClient = new Client(szWorkingDirectory,
                         szConfigDirectory,
                         szTempDirectory,
                         szEntityURL);
                          
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
                                             g_pClient->GetFileManager(),
                                             g_pClient->GetCredentialsManager(),
                                             g_pClient->GetAccessTokenCopy(),
                                             *(g_pClient->GetEntity()),
                                             g_pClient->GetTempDirectory(),
                                             g_pClient->GetWorkingDirectory(),
                                             g_pClient->GetConfigDirectory());

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
    if(status == ret::A_OK) 
        int status = app::RequestAppAuthorizationURL(*g_pApp, 
                                                     szEntityUrl, 
                                                     g_AuthorizationURL);

    if(g_pApp) {
        delete g_pApp;
        g_pApp = NULL;
    }

    return status;
}

// Move to apputils
int RequestUserAuthorizationDetails(const char* szEntityUrl,
                                    const char* szCode, 
                                    const char* szConfigDirectory)
{
    int status = ret::A_OK;
    if(!g_pApp)
        g_pApp = new TentApp();                                                

    status = app::RequestUserAuthorizationDetails(*g_pApp, 
                                                  szEntityUrl, 
                                                  szCode, 
                                                  szConfigDirectory);

    if(g_pApp) {
        delete g_pApp;
        g_pApp = NULL;
    }

    return status;
}

const char* GetAuthorizationURL() {
    return g_AuthorizationURL.c_str();
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

    Profile* prof = g_pClient->GetEntity()->GetFrontProfile();
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
                            g_pClient->GetCredentialsManager()->SetMasterKey(masterKey);
                            // Set the phrase key
                            g_pClient->SetPhraseKey(phraseKey);
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

        if(!g_pClient->GetEntity()->HasAtticProfileMasterKey() || override) {
            // Register a new passphrase.
            std::string key;
            // Enter passphrase to generate key.
            g_pClient->GetCredentialsManager()->GenerateMasterKey(key); // Generate random master key

            std::string recoverykey;

            std::cout<<" MASTER KEY : " << key << std::endl;
            status = pass::RegisterPassphraseWithAttic(std::string(szPass), 
                                                       key, // master key
                                                       g_pClient,
                                                       recoverykey);
             if(status == ret::A_OK) {
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
        g_pClient->LoadEntity(true);

        std::string salt;
        g_pClient->GetPhraseToken()->GetSalt(salt);

        std::cout<<" entering pass ... " << std::endl;
        std::string phraseKey;
        status = g_pClient->GetCredentialsManager()->EnterPassphrase(szPass, salt, phraseKey); // Enter passphrase to generate key.

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
            g_pClient->GetCredentialsManager()->GetMasterKeyCopy(mk);

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

    return status;
}

int EnterRecoveryKey(const char* szRecovery) {

    int status = IsLibInitialized(false);
    if(!szRecovery) status = ret::A_FAIL_INVALID_CSTR;
    if(status == ret::A_OK) {

        std::string recovery_key(szRecovery);
        status = g_pClient->LoadEntity(true);
        if(status == ret::A_OK) {
            std::string masterkey;
            status = pass::EnterRecoveryKey(recovery_key, 
                                            g_pClient->GetCredentialsManager(), 
                                            *(g_pClient->GetEntity()),
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

        if(g_pClient->GetCredentialsManager()) {
            MasterKey mk;
            std::string masterkey;
            g_pClient->GetCredentialsManager()->GetMasterKeyCopy(mk);
            mk.GetMasterKey(masterkey);
            status = pass::RegisterRecoveryQuestionsWithAttic(masterkey, 
                                                              g_pClient->GetCredentialsManager(),
                                                              *(g_pClient->GetEntity()), 
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

        if(g_pClient->GetCredentialsManager()) {
            std::string mkOut;
            status = pass::EnterRecoveryQuestions(g_pClient->GetCredentialsManager(),
                                            *(g_pClient->GetEntity()),
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
    if(g_pClient->GetPhraseToken()->IsPhraseKeyEmpty()) {
        // "Enter Password"
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }
    else {
        std::string phraseKey, salt;
        g_pClient->GetPhraseToken()->GetPhraseKey(phraseKey);
        g_pClient->GetPhraseToken()->GetSalt(salt);
        status = DecryptMasterKey(phraseKey, salt);
    }   

    return status;
}

int IsLibInitialized(bool checkPassphrase) {
    int status = ret::A_OK;
    if(!g_pClient)
        return ret::A_FAIL_INVALID_CLIENT;

    if(checkPassphrase && g_pClient->GetPhraseToken()->IsPhraseKeyEmpty()) {
        g_bEnteredPassphrase = false;
        status = ret::A_FAIL_NEED_ENTER_PASSPHRASE;
    }

    if(!g_pClient->GetTentApp()) status = ret::A_FAIL_INVALID_APP_INSTANCE;
    if(!g_pClient->GetCredentialsManager()) status = ret::A_FAIL_INVALID_CREDENTIALSMANAGER_INSTANCE;
    if(!g_pClient->GetFileManager()) status = ret::A_FAIL_INVALID_FILEMANAGER_INSTANCE; 
    //if(!g_bLibInitialized) status = ret::A_FAIL_LIB_INIT;

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

