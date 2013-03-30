#ifndef SYNCSTRATEGY_H_
#define SYNCSTRATEGY_H_
#pragma once

#include <string>
#include "httpstrategy.h"

namespace attic { 

class SyncStrategy : public HttpStrategyInterface {
public:
    SyncStrategy();
    ~SyncStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                const std::string& entityApiRoot, 
                const std::string& filepath, 
                Response& out);
};

}//namespace
#endif

