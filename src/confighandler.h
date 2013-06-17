#ifndef CONFIGHANDLER_H_
#define CONFIGHANDLER_H_
#pragma once

#include <string>
#include "entity.h"
#include "post.h"

namespace attic { 

class FileManager;

class ConfigHandler {
public:
    ConfigHandler(FileManager* fm);
    ~ConfigHandler();


    bool CreateConfigPost(const Entity& ent, const AccessToken* at, Post& out);
    bool UpdateConfigPost(const Entity& ent, const AccessToken* at, Post& post);
    bool RetrieveConfigPost(const Entity& ent, const AccessToken* at, Post& out);
    bool LoadConfigPost(const Entity& ent, const AccessToken* at, Post& in);
    int GetConfigPostCount(const Entity& ent, const AccessToken* at);
private:
    FileManager* file_manager_;

};

} // namespace

#endif

