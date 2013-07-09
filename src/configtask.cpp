#include "configtask.h"

namespace attic { 

ConfigTask::ConfigTask(FileManager* fm,
                       CredentialsManager* cm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context) 
                       :
                       TentTask(Task::CONFIG,
                                fm,
                                cm,
                                at,
                                entity,
                                context)
{
}

ConfigTask::~ConfigTask() {}

void ConfigTask::RunTask() {
    int status = ret::A_OK;
    std::string operation , path;
    context_.get_value("operation", operation);
    context_.get_value("directory_path", path);

    if(operation == "ADD_ROOT_DIRECTORY") {
        AddRootDirectory(path);
    }
    else if(operation == "UNLINK_ROOT_DIRECTORY") {
        UnlinkRootDirectory(path);
    }
    else if(operation == "REMOVE_ROOT_DIRECTORY") {
        RemoveRootDirectory(path);
    }
    Callback(status, operation);
    SetFinishedState();
}


bool ConfigTask::AddRootDirectory(const std::string& directory_path) {
    bool ret = true;
    return ret;
}

bool ConfigTask::UnlinkRootDirectory(const std::string& directory_path) {
    bool ret = true;
    return ret;
}

bool ConfigTask::RemoveRootDirectory(const std::string& directory_path) {
    bool ret = true;
    return ret;
}


} // namespace
