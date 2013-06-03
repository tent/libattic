#include "libattic.h"

#include <string>
#include "atticservice.h"
#include "callbackhandler.h"
#include "passphrase.h"
#include "configmanager.h"
#include "apputils.h"
#include "clientutils.h"



static attic::CallbackHandler      g_CallbackHandler;          // move to service
static attic::AtticService         attic_service; 

static std::string g_AuthorizationURL;// move to client
static bool g_bEnteredPassphrase = false;

int IsLibInitialized(bool checkPassphrase = true); // TODO :: remove

#include <cstdio> // REMOVE
//////// API start
int InitLibAttic(unsigned int threadCount) {
    freopen( "log.txt", "w", stderr ); // REMOVE
    int status = attic_service.start(); 
    return status;
}

int ShutdownLibAttic(void (*callback)(int, void*)) {
    int status = attic_service.stop();
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
    if(!szEntityUrl) return attic::ret::A_FAIL_INVALID_CSTR;
    if(!szCode) return attic::ret::A_FAIL_INVALID_CSTR;
    if(!szConfigDirectory) return attic::ret::A_FAIL_INVALID_CSTR;

    status = attic::app::RequestUserAuthorizationDetails(szEntityUrl,
                                                         szCode, 
                                                         szConfigDirectory);
    attic::event::EventSystem::instance()->ProcessEvents();

    return status;
}
int Discover(const char* szEntityurl) {
    int status = attic::ret::A_OK;
    if(!szEntityurl) return attic::ret::A_FAIL_INVALID_CSTR;

    attic::Entity ent;
    status = attic::client::Discover(szEntityurl, NULL, ent);

    return status;
}

int CreateFolder(const char* szFolderpath) {
    if(!szFolderpath) return attic::ret::A_FAIL_INVALID_CSTR;
    std::cout<<" CREATE FOLDER : " << szFolderpath << std::endl;
    int status = attic_service.CreateFolder(szFolderpath);
    return status;
}

int DeleteFolder(const char* szFolderpath) {
    if(!szFolderpath) return attic::ret::A_FAIL_INVALID_CSTR;
    std::cout<<" DELETE FOLDER : " << szFolderpath << std::endl;
    int status = attic_service.DeleteFolder(szFolderpath);
    return status;
}

int RenameFolder(const char* szOldFolderpath, const char* szNewFolderpath) {
    if(!szOldFolderpath) return attic::ret::A_FAIL_INVALID_CSTR;
    if(!szNewFolderpath) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = attic_service.RenameFolder(szOldFolderpath, szNewFolderpath);
    return status;
}

int PushFile(const char* szFilepath) {
    if(!szFilepath) return attic::ret::A_FAIL_INVALID_CSTR;
    std::cout << " PUSHING FILE : " << szFilepath << std::endl;
    int status = attic_service.UploadFile(szFilepath);
    return status;
}

int PullFile(const char* szFilepath) {
    if(!szFilepath) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = attic_service.DownloadFile(szFilepath);
    return attic::ret::A_OK;
}

int DeleteFile(const char* szFilepath) {
    if(!szFilepath) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = attic_service.MarkFileDeleted(szFilepath);
    return status;
}

int RenameFile(const char* szOldFilepath, const char* szNewFilepath) {
    if(!szOldFilepath) return attic::ret::A_FAIL_INVALID_CSTR;
    if(!szNewFilepath) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = attic_service.RenameFile(szOldFilepath, szNewFilepath);
    return status;
}

int PollFiles(void) {
    return attic_service.EnablePolling();
}


int GetFileHistory(const char* szFilepath, void(*callback)(int, const char*, int, int)) {
    if(!szFilepath) return attic::ret::A_FAIL_INVALID_CSTR;

    attic::TaskDelegate* del = g_CallbackHandler.RegisterFileHistoryCallback(callback);
    int status = attic_service.GetFileHistory(szFilepath, del);

    return status;
}


int RegisterPassphrase(const char* szPass) {
    if(!szPass) return attic::ret::A_FAIL_INVALID_CSTR;
    int status = IsLibInitialized(false);
    if(status == attic::ret::A_OK) {
        status = attic::ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        attic::pass::Passphrase ps(attic_service.client()->entity(), attic_service.client()->access_token());
        // Generate Master Key
        std::string master_key;
        attic_service.credentials_manager()->GenerateMasterKey(master_key); // Generate random master key

        std::string passphrase(szPass);
        std::cout<<" REGISTERING PASSPHRASE : " << szPass << std::endl;
        std::cout<<" TOSTR : "<< passphrase << std::endl;
        std::cout<<" LEN : " << passphrase.size() << std::endl;
        std::string recovery_key;
        status = ps.RegisterPassphrase(passphrase, master_key, recovery_key, false);

        if(status == attic::ret::A_OK) {
            attic::event::RaiseEvent(attic::event::Event::RECOVERY_KEY, recovery_key, NULL);
            status = EnterPassphrase(szPass);
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
        attic::pass::Passphrase ps(attic_service.client()->entity(), attic_service.client()->access_token());

        std::string passphrase(szPass);
        std::cout<<" PASSED IN : " << szPass << std::endl;
        std::cout<<" TOSTR : "<< passphrase << std::endl;
        std::cout<<" LEN : " << passphrase.size() << std::endl;
        std::string master_key;
        attic::PhraseToken pt;
        status = ps.EnterPassphrase(szPass, pt, master_key);

        if(status == attic::ret::A_OK) {
            attic_service.client()->set_phrase_token(pt);
            attic_service.credentials_manager()->set_master_key(master_key);
            attic_service.client()->SavePhraseToken();
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
        attic::pass::Passphrase ps(attic_service.client()->entity(), attic_service.client()->access_token());

        std::cout<<" Changing passphrase " << std::endl;
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
        attic::pass::Passphrase ps(attic_service.client()->entity(), attic_service.client()->access_token());

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
                            {"one"}, 
                            {"two"},
                            {"three"}
                           };
    return t;
}

int RegisterQuestionAnswerKey(const char* q1, 
                              const char* q2, 
                              const char* q3, 
                              const char* a1, 
                              const char* a2, 
                              const char* a3) {

    int status = IsLibInitialized(false);
    /*
    int status = attic_service.client()->LoadEntity(true);
    if(status == attic::ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(attic_service.client()->credentials_manager()) {
            attic::MasterKey mk;
            std::string masterkey;
            attic_service.client()->credentials_manager()->GetMasterKeyCopy(mk);
            mk.GetMasterKey(masterkey);
            attic::Entity ent = attic_service.client()->entity();
            status = attic::pass::RegisterRecoveryQuestionsWithAttic(masterkey, 
                                                              attic_service.client()->credentials_manager(),
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

    */
    return status;
}

int EnterQuestionAnswerKey(const char* q1, 
                           const char* q2, 
                           const char* q3, 
                           const char* a1, 
                           const char* a2, 
                           const char* a3) {

    int status = attic::ret::A_OK;
    /*
    int status = attic_service.client()->LoadEntity(true);
    if(status == attic::ret::A_OK) {
        std::string questionOne(q1);
        std::string questionTwo(q2);
        std::string questionThree(q3);
        std::string answerOne(a1);
        std::string answerTwo(a2);
        std::string answerThree(a3);

        if(attic_service.client()->credentials_manager()) {
            std::string mkOut;
            attic::Entity ent = attic_service.client()->entity();
            status = attic::pass::EnterRecoveryQuestions(attic_service.client()->credentials_manager(),
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
                                                           attic_service.client(),
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
*/
    return status;
}

int PhraseStatus() {
    int status = attic::ret::A_OK;

    if(!g_bEnteredPassphrase)
        status = attic::ret::A_FAIL_NEED_ENTER_PASSPHRASE;

    return status;
}

int IsLibInitialized(bool checkPassphrase) {
    int status = attic::ret::A_OK;
    if(!attic_service.running()) status = attic::ret::A_FAIL_LIB_INIT;
    return status;
}

int GetFileList(void(*callback)(int, char**, int, int)) {
    int status = attic::ret::A_OK;
    if(!callback)
        status = attic::ret::A_FAIL_INVALID_PTR;

    if(status == attic::ret::A_OK) {
        status = IsLibInitialized();
        attic::TaskDelegate* del = g_CallbackHandler.RegisterManifestCallback(callback);
        status = attic_service.QueryManifest(del);
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
    int status = attic::ret::A_OK;
    attic::event::RaiseEvent(attic::event::Event::PAUSE, "", NULL);
    return status;
}

int Resume(void) {
    int status = attic::ret::A_OK;
    attic::event::RaiseEvent(attic::event::Event::RESUME, "", NULL);
    // TODO :: re-implement this
    //attic_service.task_manager()->ScanAtticFolder(NULL);
    return status;
}

int ScanAtticFolder() { 
    int status = attic::ret::A_OK;
    // TODO :: re-implement
    //attic_service.task_manager()->ScanAtticFolder(NULL); k
    return status;
}

void SetConfigValue(const char* szKey, const char* szValue) {
    std::string key(szKey);
    std::string value(szValue);
    attic::ConfigManager::GetInstance()->SetValue(key, value);
}

int HasCredentialsPost() {
    int status = IsLibInitialized();
    if(status == attic::ret::A_OK) {
        attic::pass::Passphrase ps(attic_service.client()->entity(), 
                                   attic_service.client()->access_token());
        bool retval = ps.HasCredentialsPost();
        if(!retval)
            status = attic::ret::A_FAIL;
    }
    return status;
}

