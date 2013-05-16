#ifndef ATTICSERVICE_H_
#define ATTICSERVICE_H_
#pragma once

#include <string>
#include "callbackhandler.h"
#include "atticclient.h"
#include "servicemanager.h"
#include "taskmanager.h"
#include "threading.h"

namespace attic {

class AtticService {
    void LoadConfigValues();
    void ValidateDirectories();
public:
    AtticService();
    ~AtticService();

    int start();
    int stop();

    int UploadFile();
    int DownloadFile();
    int MarkFileDeleted();
    int RenameFile();
    int RenameFolder();
    int EnablePolling();
    int DisablePolling();

    int RegisterPassphrase();
    int EnterPassphrase();
    int ChangePassphrase();
    int EnterRecoveryKey();
private:
    bool running_;

    ServiceManager*     service_manager_;
    TaskManager*        task_manager_;
    ThreadManager*      thread_manager_;
    Client*             client_;

    CallbackHandler callback_handler_;

    std::string working_dir_;
    std::string config_dir_;
    std::string temp_dir_;
    std::string entity_url_;
};

}  // namespace
#endif

