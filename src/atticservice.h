#ifndef ATTICSERVICE_H_
#define ATTICSERVICE_H_
#pragma once

class AtticService {
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

};

#endif

