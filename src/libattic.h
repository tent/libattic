

#ifndef LIBATTIC_H_
#define LIBATTIC_H_


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


int TestQuery();

int ShutdownAppInstance();

int InitializeFileManager();

int ShutdownFileManager();

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
int PushFile(const char* szFilePath);

int PushFileTask(const char* szFilePath, void (*callback)(int, void*) );

// Pullfile from tent
int PullFile(const char* szFilePath);

// Test method
int PullFileTask(const char* szFilePath, void (*callback)(int, void*));

// Delete a file
int DeleteFile(const char* szFileName);

// Pull All files in manifest
int PullAllFiles();

// TODO :: filter attic posts, and construct manifest from that if none is present
//         compare data and make sure both are up to data

int SetWorkingDirectory(const char* szDir);
int SetConfigDirectory(const char* szDir);
int SetTempDirectory(const char* szDir);

int SetEntityUrl(const char* szUrl);

const char* GetWorkingDirectory();
const char* GetConfigDirectory();
const char* GetEntityUrl();

int GetAtticPostCount();
int SyncAtticPosts();

int SaveChanges();
}
#endif

