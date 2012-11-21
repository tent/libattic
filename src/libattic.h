

#ifndef LIBATTIC_H_
#define LIBATTIC_H_

typedef unsigned int TENTAPP;

int StartupAppInstance(const char* szAppName, const char* szAppDescription, const char* szUrl, const char* szIcon, char* redirectUris[], unsigned int uriCount, char* scopes[], unsigned int scopeCount);

int ShutdownAppInstance();

// Pass the uri to the api path for apps (ex "https://test.tent.is/tent/app")
int RegisterApp(const char* szPostPath);

// Pass the api root of the entity (ex "https://test.tent.is/tent/")
int RequestAppAuthorizationURL(const char* szApiRoot);

const char* GetAuthorizationURL();

int RequestUserAuthorizationDetails(const char* szCode);

#endif

