#include "atticservice.h"

#include "utils.h"
#include "configmanager.h"
#include "constants.h"
#include "filesystem.h"
#include "errorcodes.h"

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

AtticService::~AtticService() {

}

int AtticService::start() {
    int status = ret::A_OK;
    utils::SeedRand();
    LoadConfigValues();
    status = ValidateDirectories();
    if(status == ret::A_OK){
        // Initialize File Manager
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
    status = ShutdownThreadManager();
    status = ShutdownTaskArbiter();
    status = ShutdownTaskManager();
    status = ShutdownServiceManager();
    status = ShutdownTaskManager();
    status = ShutdownClient();
    status = ShutdownCredentialsManager();
    status = ShutdownFileManager();
    ConfigManager::GetInstance()->Shutdown();
    running_ = false;
    return status;
}

void AtticService::LoadConfigValues() {
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigWorkingDir, working_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigConfigDir, config_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigTempDir, temp_dir_);
    ConfigManager::GetInstance()->GetValue(cnst::g_szConfigEntityURL, entity_url_);
}

int AtticService::ValidateDirectories() {
    int status = ret::A_OK;
    if(!working_dir_.empty() && !config_dir_.empty() && !temp_dir_.empty()) {
        // TODO:: make sure these are all absolute paths
        fs::CreateDirectory(working_dir_);
        fs::CreateDirectory(config_dir_);
        fs::CreateDirectory(temp_dir_);
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

// TODO :: REFACTOR get rid of credentials manager
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
        status = thread_manager_->Initialize(12); 
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


}// namespace
