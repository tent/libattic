#ifndef LIBATTIC_H_
#define LIBATTIC_H_
#pragma once

extern "C" {

/* App Registration interface :
 *      This should be done before using the lib, to generate app credentials
 *      and autorize the application for use with the given tent server
 *      There is an order to this.
 *      - StartupAppInstance
 *      - RegisterApp
 *      - RequestAppAuthorizationURL
 *      - RequestAppAuthorizationDetails
 */
int RegisterAtticApp(const char* szEntityurl,
                     const char* szAppName, 
                     const char* szAppDescription, 
                     const char* szUrl, 
                     const char* szIcon, 
                     char* redirectUris[], 
                     unsigned int uriCount, 
                     char* scopes[], 
                     unsigned int scopeCount, 
                     const char* szConfigDir);


int RequestUserAuthorizationDetails( const char* szEntityUrl, 
                                     const char* szCode,
                                     const char* szConfigDirectory); // Config Directory
const char* GetAuthorizationURL();

// Api begin
int InitLibAttic(unsigned int threadCount = 5);

int ShutdownLibAttic(void (*callback)(int, void*));

// Master Key
int EnterPassphrase(const char* szPass);
int EnterRecoveryKey(const char* szRecovery);
int RegisterPassphrase(const char* szPass, bool override = false);
int ChangePassphrase(const char* szOld, const char* szNew);
int GetPhraseStatus();


// Register for Events
// Event Type, Event Status, data
//  EventStatus:
//    - START = 0,
//    - RUNNING,                     
//    - PAUSED,
//    - DONE,
void RegisterForPullNotify(void (*callback)(int, int, const char*));
void RegisterForPushNotify(void (*callback)(int, int, const char*));
void RegisterForUploadSpeedNotify(void (*callback)(int, int, const char*));
void RegisterForDownloadSpeedNotify(void (*callback)(int, int, const char*));
void RegisterForErrorNotify(void (*callback)(int, int, const char*));
void RegisterForRecoveryKeyNotify(void (*callback)(int, int, const char*));
void RegisterForTemporaryKeyNotify(void (*callback)(int, int, const char*));
void RegisterForPauseResumeNotify(void (*callback)(int, int, const char*));



int PushFile(const char* szFilePath);
int PullFile(const char* szFilePath);
int DeleteFile(const char* szFilePath);
int PollFiles(void);

int Pause(void);
int Resume(void);

const char** GetQuestionList();

int RegisterQuestionAnswerKey(const char* q1, 
                              const char* q2, 
                              const char* q3, 
                              const char* a1, 
                              const char* a2, 
                              const char* a3);

int EnterQuestionAnswerKey(const char* q1, 
                           const char* q2, 
                           const char* q3, 
                           const char* a1, 
                           const char* a2, 
                           const char* a3);

// Returns calls back n numbers of times, with filepaths
// callback
// - status, array of char*, number in array, total number
int GetFileList(void(*callback)(int, char**, int, int));

// Once finished with the file list, pass back here for memory cleanup
int FreeFileList(char** pList, int stride);

// Current used values - see constants.h for more
// thrash_path | <filepath>
// upload_limit | <limit> (mbs)
void SetConfigValue(const char* szKey, const char* szValue);
}

#endif

