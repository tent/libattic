#ifndef ATTICSERVICE_H_
#define ATTICSERVICE_H_
#pragma once

#include <string>
//#include "callbackhandler.h"
#include "atticclient.h"
#include "servicemanager.h"
#include "taskmanager.h"
#include "taskarbiter.h"
#include "threading.h"
#include "filemanager.h"
#include "credentialsmanager.h"

namespace attic {

class AtticService {
    void LoadConfigValues();
    int ValidateDirectories();
    // Order does matter for init and shutdown sequence
    int InitializeFileManager();
    int InitializeCredentialsManager();
    int InitializeClient();
    int InitializeTaskManager();
    int InitializeServiceManager();
    int InitializeTaskArbiter();
    int InitializeThreadManager();

    int ShutdownThreadManager();
    int ShutdownTaskArbiter();
    int ShutdownServiceManager();
    int ShutdownTaskManager();
    int ShutdownClient();
    int ShutdownCredentialsManager();
    int ShutdownFileManager();
public:
    AtticService();
    ~AtticService();

    int start();
    int stop();

    int UploadFile(const std::string& filepath);
    int DownloadFile(const std::string& filepath);
    int MarkFileDeleted(const std::string& filepath);
    int RenameFile(const std::string& old_filepath, const std::string& new_filepath);
    int CreateFolder(const std::string& folderpath);
    int DeleteFolder(const std::string& folderpath);
    int RenameFolder(const std::string& old_folderpath, const std::string& new_folderpath);
    int EnablePolling();
    int DisablePolling();

    int QueryManifest(TaskDelegate* cb);
    int GetFileHistory(const std::string& filepath, TaskDelegate* cb);

    int RegisterPassphrase();
    int EnterPassphrase();
    int ChangePassphrase();
    int EnterRecoveryKey();

    bool running()                              { return running_; }
    TaskManager* task_manager()                 { return task_manager_; }
    CredentialsManager* credentials_manager()   { return credentials_manager_; }
    Client* client()                            { return client_; }
private:
    bool running_;

    ServiceManager*     service_manager_;
    TaskManager*        task_manager_;
    TaskArbiter*        task_arbiter_;
    ThreadManager*      thread_manager_;
    FileManager*        file_manager_;
    CredentialsManager* credentials_manager_;
    Client*             client_;

//    CallbackHandler callback_handler_;

    // File paths should all be absolute paths
    std::string working_dir_;
    std::string config_dir_;
    std::string temp_dir_;
    std::string entity_url_;
};

}  // namespace
#endif

