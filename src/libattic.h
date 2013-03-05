#ifndef LIBATTIC_H_
#define LIBATTIC_H_
#pragma once


extern "C"
{

/* App Registration interface :
 *      This should be done before using the lib, to generate app credentials
 *      and autorize the application for use with the given tent server
 *      There is an order to this.
 *      - StartupAppInstance
 *      - RegisterApp
 *      - RequestAppAuthorizationURL
 *      - RequestAppAuthorizationDetails
 */

int StartupAppInstance( const char* szAppName, 
                        const char* szAppDescription, 
                        const char* szUrl, 
                        const char* szIcon, 
                        char* redirectUris[], 
                        unsigned int uriCount, 
                        char* scopes[], 
                        unsigned int scopeCount);

// Pass the uri to the api path for apps (ex "https://test.tent.is/tent/app")
int RegisterApp(const char* szEntityUrl, const char* szConfigDirectory);

// Pass the api root of the entity (ex "https://test.tent.is/tent/")
// * Must Register app successfully before proceeding to this step
int RequestAppAuthorizationURL(const char* szEntityUrl);

int RequestUserAuthorizationDetails( const char* szEntityUrl, 
                                     const char* szCode,
                                     const char* szConfigDirectory); // Config Directory

const char* GetAuthorizationURL();

const char* GetEntityApiRoot(const char* szEntityUrl);

// Api begin
int InitLibAttic( const char* szWorkingDirectory, 
                  const char* szConfigDirectory,
                  const char* szTempDirectory,
                  const char* szLogDirectory,
                  const char* szEntityURL,
                  unsigned int threadCount = 2);

int ShutdownLibAttic(void (*callback)(int, void*));

// Master Key
int EnterPassphrase(const char* szPass);
int RegisterPassphrase(const char* szPass, bool override = false);
int ChangePassphrase(const char* szOld, const char* szNew);
int GetPhraseStatus();



// Save the app in json to a file (Just a utility you probably don't
// want to use this in production)
int SaveAppToFile();

// Load the app in json from a file (Just a utility you probably don't
// want to use this in production)
int LoadAppFromFile();
int LoadAccessToken();

// Pushfile to tent
int PushFile(const char* szFilePath, void (*callback)(int, void*));

// Pullfile from tent
int PullFile(const char* szFilePath, void (*callback)(int, void*));

// Delete a file
int DeleteFile(const char* szFilePath, void (*callback)(int, void*));

// Pull All files in manifest
int PullAllFiles(void (*callback)(int, void*)); // Pull into lib, don't expose

// TODO :: These two methods are just temporarily exposed, to be used internally only
int SyncFiles(void (*callback)(int, void*));
int PollFiles(void (*callback)(int, void*));

int SetEntityUrl(const char* szUrl);

// Status Methods
int GetCurrentTasks(void (*callback)(char* pArr, int count));
int GetActivePushTaskCount();
int GetActivePullTaskCount();
int GetActiveUploadSpeed();
int GetActiveDownloadSpeed();

// Utility
const char* GetWorkingDirectory();
const char* GetConfigDirectory();
const char* GetEntityUrl();

// Returns calls back n numbers of times, with filepaths
// callback
// - status, array of char*, number in array, total number
int GetFileList(void(*callback)(int, char**, int, int));

// Once finished with the file list, pass back here for memory cleanup
int FreeFileList(char** pList, int stride);

// Utility function <- vince, make a temporary button of some sort for this, to nuke your account
int DeleteAllPosts(void (*callback)(int, void*));

// TEMP FUNCTIONS REMOVE THESE FOR TESTING PURPOSES ONLY
class FileManager;
FileManager* GetFileManager();
}

#endif

