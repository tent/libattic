

#ifndef LIBATTIC_H_
#define LIBATTIC_H_

typedef unsigned int TENTAPP;

int StartupAppInstance(const char* szAppName, const char* szAppDescription, const char* szUrl, const char* szIcon, char* redirectUris[], unsigned int uriCount, char* scopes[], unsigned int scopeCount);

int ShutdownAppInstance();

// Pass the uri to the api path for apps (ex "https://test.tent.is/tent/app")
int RegisterApp(const char* szPostPath);

// Pass the api root of the entity (ex "https://test.tent.is/tent/")
// * Must Register app successfully before proceeding to this step
int RequestAppAuthorizationURL(const char* szApiRoot);

const char* GetAuthorizationURL();

int RequestUserAuthorizationDetails(const char* szApiRoot, const char* szCode);

// Save the app in json to a file (Just a utility you probably don't
// want to use this in production)
int SaveAppToFile(const char* szFilePath);

// Load the app in json from a file (Just a utility you probably don't
// want to use this in production)
int LoadAppFromFile(const char* szFilePath);

int LoadAccessToken(const char* szFilePath);

int PostFile(const char* szUrl, const char* szFilePath);

int GetFile(const char* szUrl, const char* szPostID);

#endif

