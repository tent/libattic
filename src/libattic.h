#ifndef LIBATTIC_H_
#define LIBATTIC_H_
#pragma once


extern "C"
{

int StartupAppInstance( const char* szAppName, 
                        const char* szAppDescription, 
                        const char* szUrl, 
                        const char* szIcon, 
                        char* redirectUris[], 
                        unsigned int uriCount, 
                        char* scopes[], 
                        unsigned int scopeCount);


int InitLibAttic( const char* szWorkingDirectory, 
                  const char* szConfigDirectory,
                  const char* szTempDirectory,
                  const char* szEntityURL,
                  unsigned int threadCount = 2);

int ShutdownLibAttic();

// Master Key
int EnterPassphrase(const char* szPass);
int RegisterPassphrase(const char* szPass, bool override = false);
int ChangePassphrase(const char* szOld, const char* szNew);
int GetPhraseStatus();

// Pass the uri to the api path for apps (ex "https://test.tent.is/tent/app")
int RegisterApp(const char* szPostPath);

// Pass the api root of the entity (ex "https://test.tent.is/tent/")
// * Must Register app successfully before proceeding to this step
int RequestAppAuthorizationURL(const char* szApiRoot);

const char* GetAuthorizationURL();

int RequestUserAuthorizationDetails(const char* szApiRoot, const char* szCode);

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
int DeleteFile(const char* szFileName, void (*callback)(int, void*));

// Pull All files in manifest
int PullAllFiles();

// Sync attic metadata
int SyncAtticMetaData(void (*callback)(int, void*));

// Sync with attic posts
int SyncAtticPostsMetaData(void (*callback)(int, void*));

int SetEntityUrl(const char* szUrl);

const char* GetWorkingDirectory();
const char* GetConfigDirectory();
const char* GetEntityUrl();

int GetAtticPostCount();
int SyncAtticPosts();

int SaveChanges();

int TestQuery();

// Utility function
int DeleteAllPosts();

// TEMP FUNCTIONS REMOVE THESE FOR TESTING PURPOSES ONLY
class FileManager;
FileManager* GetFileManager();
}

#endif

