#include "atticservice.h"

#include "utils.h"
#include "configmanager.h"
#include "constants.h"
#include "filesystem.h"
#include "errorcodes.h"
#include "passphrase.h"

namespace attic {

AtticService::AtticService() {
    service_manager_ = NULL;
    task_manager_ = NULL;
    task_arbiter_ = NULL;
    thread_manager_ = NULL;
    file_manager_ = NULL;
    credentials_manager_ = NULL;
    client_ = NULL;

    running_ = false;
}

AtticService::~AtticService() {}

int AtticService::start() {
    int status = ret::A_OK;
    utils::SeedRand();
    LoadConfigValues();
    status = ValidateDirectories();
    if(status == ret::A_OK){
        // Initialize File Manager
        status = InitializeConnectionManager();     if(status != ret::A_OK) return status;
        status = InitializeFileManager();           if(status != ret::A_OK) return status;
        status = InitializeCredentialsManager();    if(status != ret::A_OK) return status;
        status = InitializeClient();                if(status != ret::A_OK) return status;
        status = InitializeTaskManager();           if(status != ret::A_OK) return status;
        status = InitializeServiceManager();        if(status != ret::A_OK) return status;
        status = InitializeTaskArbiter();           if(status != ret::A_OK) return status;
        status = InitializeThreadManager();         if(status != ret::A_OK) return status;

    }
    running_ = true;
    return status;
}

int AtticService::stop() {
    int status = ret::A_OK;
    std::ostringstream err;
    err << " Attic Service stop [fail] : ";
    status = ShutdownThreadManager();       if(status!=ret::A_OK) { err <<"thm : " << status; }
    status = ShutdownTaskArbiter();         if(status!=ret::A_OK) { err <<"ta : " << status; }
    status = ShutdownTaskManager();         if(status!=ret::A_OK) { err <<"tm : " << status; }
    status = ShutdownServiceManager();      if(status!=ret::A_OK) { err <<"sm : " << status; }
    status = ShutdownClient();              if(status!=ret::A_OK) { err <<"cl : " << status; }
    status = ShutdownCredentialsManager();  if(status!=ret::A_OK) { err <<"cm : " << status; }
    status = ShutdownFileManager();         if(status!=ret::A_OK) { err <<"fm : " << status; }
    status = ShutdownConnectionManager();   if(status!=ret::A_OK) { err <<"cm : " << status; }
    ConfigManager::GetInstance()->Shutdown();if(status!=ret::A_OK) { err <<"cnm : " << status; }
    std::cout<< err.str() << std::endl;
    running_ = false;
    return status;
}

int AtticService::UploadFile(const std::string& filepath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->UploadFile(filepath, NULL);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::DownloadFile(const std::string& filepath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->DownloadFile(filepath, NULL);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::MarkFileDeleted(const std::string& filepath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->DeleteFile(filepath, NULL);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}
 
int AtticService::RenameFile(const std::string& old_filepath, const std::string& new_filepath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->RenameFile(old_filepath, new_filepath);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::CreateFolder(const std::string& folderpath) { 
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->CreateFolder(folderpath, NULL);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::DeleteFolder(const std::string& folderpath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->DeleteFolder(folderpath, NULL);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::RenameFolder(const std::string& old_folderpath, const std::string& new_folderpath) {
    int status = ret::A_OK;
    if(running_) {
        if(IsMasterKeyValid())
            task_manager_->RenameFolder(old_folderpath, new_folderpath);
        else 
            status = ret::A_FAIL_INVALID_MASTERKEY;
    }
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

// TODO :: toggling polling should be event driven, TODO :: fix this
int AtticService::BeginPolling() {
    int status = ret::A_OK;
    if(running_)
        task_manager_->PollFiles(NULL);
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::Pause() {
    int status = ret::A_OK;
    if(running_)
        event::RaiseEvent(event::Event::PAUSE, "", NULL);
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::Resume() {
    int status = ret::A_OK;
    if(running_)
        event::RaiseEvent(event::Event::RESUME, "", NULL);
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::QueryManifest(TaskDelegate* cb) {
    int status = ret::A_OK;
    if(running_)
        task_manager_->QueryManifest(cb);
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

int AtticService::GetFileHistory(const std::string& filepath, TaskDelegate* cb) {
    int status = ret::A_OK;
    if(running_)
        task_manager_->GetFileHistory(filepath, cb);
    else 
        status = ret::A_FAIL_SERVICE_NOT_RUNNING;
    return status;
}

void AtticService::LoadConfigValues() {
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigWorkingDir, working_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigConfigDir, config_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigTempDir, temp_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigEntityURL, entity_url_);
    std::cout<< " loading config values : " << std::endl;
    std::cout<< ConfigManager::GetInstance()->toString() << std::endl;
}

int AtticService::ValidateDirectories() {
    int status = ret::A_OK;
    if(!working_dir_.empty() && !config_dir_.empty() && !temp_dir_.empty()) {
        // TODO:: make sure these are all absolute paths
        fs::CreateDirectory(working_dir_);
        fs::CreateDirectory(config_dir_);
        fs::CreateDirectory(temp_dir_);

        fs::GetCanonicalPath(working_dir_, working_dir_);
        fs::GetCanonicalPath(config_dir_, config_dir_);
        fs::GetCanonicalPath(temp_dir_, temp_dir_);
    }
    else {
        status = ret::A_FAIL_VALIDATE_DIRECTORY;
    }
    return status;
}

int AtticService::InitializeFileManager() {
    int status = ret::A_OK;
    if(!file_manager_) {
        file_manager_ = new FileManager();
        status = file_manager_->Initialize(config_dir_,
                                           working_dir_,
                                           temp_dir_);
    }
    return status;
}

// TODO :: REFACTOR (possibly) get rid of credentials manager
int AtticService::InitializeCredentialsManager() {
    int status = ret::A_OK;
    if(!credentials_manager_) {
        credentials_manager_ = new CredentialsManager();
        credentials_manager_->SetConfigDirectory(config_dir_);
        status = credentials_manager_->Initialize();
        if(status == ret::A_OK) {
            status = credentials_manager_->LoadAccessToken();
        }
    }
    return status;
}

int AtticService::InitializeClient() {
    int status = ret::A_OK;
    AccessToken at;
    credentials_manager_->GetAccessTokenCopy(at);
    client_ = new Client(working_dir_,
                         config_dir_,
                         temp_dir_,
                         entity_url_,
                         at);
    status = client_->Initialize();
    return status;
}

int AtticService::InitializeTaskManager() {
    int status = ret::A_OK;
    if(!task_manager_) {
        AccessToken at;
        credentials_manager_->GetAccessTokenCopy(at);
        task_manager_ = new TaskManager(file_manager_, 
                                        credentials_manager_, 
                                        at, 
                                        client_->entity(),
                                        temp_dir_,
                                        working_dir_,
                                        config_dir_);
        status = task_manager_->Initialize();
    }
    return status;
}

int AtticService::InitializeServiceManager() {
    int status = ret::A_OK;
    if(!service_manager_) {
        service_manager_ = new ServiceManager(task_manager_);
        status = service_manager_->Initialize();
    }
    return status;
}

int AtticService::InitializeTaskArbiter() {
    int status = ret::A_OK;
    if(!task_arbiter_) {
        task_arbiter_ = TaskArbiter::GetInstance();
        status = task_arbiter_->Initialize(task_manager_);
    }
    return status;
}

int AtticService::InitializeThreadManager() {
    int status = ret::A_OK;
    if(!thread_manager_) {
        AccessToken at;
        credentials_manager_->GetAccessTokenCopy(at);
        thread_manager_ = new ThreadManager(file_manager_,
                                            credentials_manager_,
                                            at,
                                            client_->entity());
        // TODO :: setup a configurable way to set the thread count
        status = thread_manager_->Initialize(20); 
    }
    return status;
}

int AtticService::InitializeConnectionManager() {
    int status = ret::A_OK;
    if(!connection_manager_) {
        connection_manager_ = ConnectionManager::instance();
        status = connection_manager_->Initialize(entity_url_);
    }
    return status;
}

int AtticService::ShutdownConnectionManager() {
    int status = ret::A_OK;
    if(connection_manager_) {
        status = connection_manager_->Shutdown();
    }
    return status;
}

int AtticService::ShutdownThreadManager() {
    int status = ret::A_OK;
    if(thread_manager_) {
        status = thread_manager_->Shutdown();
        delete thread_manager_;
        thread_manager_ = NULL;
    }
    return status;
}

int AtticService::ShutdownTaskArbiter() {
    int status = ret::A_OK;
    if(task_arbiter_) {
        // NOTE* since this is a singleton it deletes itself on shutdown
        // TODO :: eventually make the task arbiter not a singleton
        status = task_arbiter_->Shutdown();
        task_arbiter_ = NULL;
    }
    return status;
}

int AtticService::ShutdownServiceManager() {
    int status = ret::A_OK;
    if(service_manager_) {
        status = service_manager_->Shutdown();
        delete service_manager_;
        service_manager_ = NULL;
    }
    return status;
}

int AtticService::ShutdownTaskManager() {
    int status = ret::A_OK;
    if(task_manager_) {
        task_manager_->Shutdown();
        delete task_manager_;
        task_manager_ = NULL;
    }
    return status;
}

int AtticService::ShutdownClient() {
    int status = ret::A_OK;
    if(client_) {
        client_->Shutdown();
        delete client_;
        client_ = NULL;
    }
    return status;
}

int AtticService::ShutdownCredentialsManager() {
    int status = ret::A_OK;
    if(credentials_manager_) {
        status = credentials_manager_->Shutdown();
        delete credentials_manager_;
        credentials_manager_ = NULL;
    }
    return status;
}

int AtticService::ShutdownFileManager() {
    int status = ret::A_OK;
    if(file_manager_) {
        status = file_manager_->Shutdown();
        delete file_manager_;
        file_manager_ = NULL;
    }
    return status;
}

bool AtticService::IsMasterKeyValid() {
    MasterKey mk;
    credentials_manager_->GetMasterKeyCopy(mk);
    std::string key;
    mk.GetMasterKey(key);
    if(!key.empty())
        return true;
    return false;
}

int AtticService::RegisterPassphrase(const std::string& passphrase) {
    int status = ret::A_FAIL_LIB_INIT;
    if(running_) {
        status = ret::A_FAIL_REGISTER_PASSPHRASE;
        // Discover Entity, get access token
        pass::Passphrase ps(client_->entity(), client_->access_token());
        // Generate Master Key
        std::string master_key;
        credentials_manager_->GenerateMasterKey(master_key); // Generate random master key

        std::string passphrase(passphrase);
        std::cout<<" REGISTERING PASSPHRASE : " << passphrase << std::endl;
        std::cout<<" TOSTR : "<< passphrase << std::endl;
        std::cout<<" LEN : " << passphrase.size() << std::endl;
        std::string recovery_key;
        status = ps.RegisterPassphrase(passphrase, master_key, recovery_key, false);

        if(status == ret::A_OK) {
            event::RaiseEvent(event::Event::RECOVERY_KEY, recovery_key, NULL);
            status = EnterPassphrase(passphrase);
        }
    }
    return status;
}

int AtticService::EnterPassphrase(const std::string& passphrase) {
    int status = ret::A_FAIL_LIB_INIT;
    if(running_) {
        // Discover Entity, get access token
        pass::Passphrase ps(client_->entity(), client_->access_token());

        std::string passphrase(passphrase);
        std::cout<<" PASSED IN : " << passphrase << std::endl;
        std::cout<<" TOSTR : "<< passphrase << std::endl;
        std::cout<<" LEN : " << passphrase.size() << std::endl;
        std::string master_key;
        PhraseToken pt;
        status = ps.EnterPassphrase(passphrase, pt, master_key);

        if(status == ret::A_OK) {
            client_->set_phrase_token(pt);
            credentials_manager_->set_master_key(master_key);
            client_->SavePhraseToken();
        }
    }
    return status;
}

}// namespace
