#ifndef ATTICSERVICE_H_
#define ATTICSERVICE_H_
#pragma once

#include <string>
#include "atticclient.h"
#include "servicemanager.h"
#include "taskmanager.h"
#include "taskarbiter.h"
#include "threading.h"
#include "filemanager.h"
#include "credentialsmanager.h"

#include "post.h"

/* Attic Service
 *  Service to direct spawning of threads, insertion of tasks, and management of
 *  subsystems, allowing communication into the lib.
 */
namespace attic {
    class Polling;

class AtticService {
    bool IsMasterKeyValid();
    void LoadConfigValues();
    int ValidateDirectories();
    // Order does matter for init and shutdown sequence
    int InitializeConnectionManager();
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
    int ShutdownConnectionManager();

    void ValidateTimeOffset();
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
    int BeginPolling();
    int Pause(); // Polling
    int Resume(); // Pollling

    int QueryManifest(TaskDelegate* cb);
    int GetFileHistory(const std::string& filepath, TaskDelegate* cb);

    int RegisterPassphrase(const std::string& pass);
    int EnterPassphrase(const std::string& pass);
    int ChangePassphrase(const std::string& old_passphrase, const std::string& new_passphrase);
    int EnterRecoveryKey(const std::string& recovery_key);


    // Directory methods
    int CreateWorkingDirectory(const std::string& filepath);
    int LinkWorkingDirectory(const std::string& filepath, const std::string& post_id);
    bool IsFilepathLinked(const std::string& filepath);

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
    ConnectionManager*  connection_manager_;

    Polling*            polling_;

    // File paths should all be absolute paths
    std::string working_dir_;
    std::string config_dir_;
    std::string temp_dir_;
    std::string entity_url_;
};

}  // namespace
#endif

