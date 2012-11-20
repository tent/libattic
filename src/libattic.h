

#ifndef LIBATTIC_H_
#define LIBATTIC_H_

typedef unsigned int TENTAPP;

TENTAPP StartupAppInstance(const char* szAppName, const char* szAppDescription, const char* szUrl, const char* szIcon, char* redirectUris[], char* scopes[]);


#endif

