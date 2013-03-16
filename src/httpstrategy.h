#ifndef HTTPSTRATEGY_H_
#define HTTPSTRATGEY_H_
#pragma once

#include <string>
#include "response.h"
#include "filemanager.h"
#include "credentialsmanager.h"

class HttpStrategyInterface {
public:
    virtual void Execute(FileManager* pFileManager,
                         CredentialsManager* pCredentialsManager,
                         const std::string& entityApiRoot, 
                         const std::string& filepath, 
                         Response& out)=0;
};

#endif

