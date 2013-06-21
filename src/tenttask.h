#ifndef TENTTASK_H_
#define TENTTASK_H_
#pragma once

#include <map>
#include <string>

#include "task.h"
#include "entity.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskdelegate.h"

namespace attic { 

class TentTask : public Task {
public:
    TentTask(Task::TaskType type,
             FileManager* fm, 
             CredentialsManager* cm,
             const AccessToken& at,
             const Entity& entity,
             const TaskContext& context)
             : 
             Task(context, type) {
        file_manager_          = fm;
        credentials_manager_   = cm;
        access_token_          = at;
        entity_                = entity;

        context.get_value("filepath", filepath_);
        context.get_value("temp_dir", temp_directory_);
        context.get_value("working_dir", working_directory_);
        context.get_value("config_dir", config_directory_);

        callback_delegate_ = context.delegate();
    }

    virtual void SetFinishedState() {
        Task::SetFinishedState();
    }                                                                       

    virtual ~TentTask() {
        file_manager_          = NULL;
        credentials_manager_   = NULL;
        callback_delegate_     = NULL;
    }

    /*
    virtual void RunTask() {}
    */

    const AccessToken& access_token() const { return access_token_; }
    const Entity* entity() const { return &entity_; }

    const std::string& filepath()           const { return filepath_; }
    const std::string& temp_directory()     const { return temp_directory_; }
    const std::string& wokring_directory()  const { return working_directory_; }
    const std::string& config_directory()   const { return config_directory_; }

    FileManager*        file_manager()        { return file_manager_; } 
    CredentialsManager* credentials_manager() { return credentials_manager_; } 

    void set_file_manager(FileManager* fm)                  { file_manager_ = fm; }
    void set_credentials_manager(CredentialsManager* cm)     { credentials_manager_ = cm; }

    void set_access_token(const AccessToken& at)                 { access_token_ = at; }
    void set_entity(const Entity& entity)                        { entity_ = entity; }
    void set_filepath(const std::string& filepath)               { filepath_ = filepath; }
    void set_temp_directory(const std::string& tempdir)          { temp_directory_ = tempdir; }
    void set_working_directory(const std::string& workingdir)    { working_directory_ = workingdir; }
    void set_config_directory(const std::string& configdir)      { config_directory_ = configdir; }

    void SetConfigValue(const std::string& key, const std::string& value) { 
        config_table_[key] = value; 
    }

    bool GetConfigValue(const std::string& key, std::string& out) {
        if(config_table_.find(key) != config_table_.end()) {
            out = config_table_[key];
            return true;
        }
        return false;
    }

protected:
    void Callback(const int code, const std::string& var) {
        if(callback_delegate_) { 
            std::cout<< "calling back : " << var << " code " << code << std::endl;
            callback_delegate_->Callback(type(), code, state(), var);
        }
    }

    std::string GetPostPath() {
        std::string post_path;
        utils::FindAndReplace(entity_.GetPreferredServer().post(), 
                              "{entity}", 
                              entity_.entity(), 
                              post_path);
        return post_path;
    }

    TaskDelegate* callback_delegate() { return callback_delegate_; }

    bool GetMasterKey(std::string& out) {
        MasterKey mKey;
        credentials_manager_->GetMasterKeyCopy(mKey);
        mKey.GetMasterKey(out);
        if(out.size())
            return true;
        return false;
    }

    bool ValidMasterKey() {
        std::string mk;
        return GetMasterKey(mk);
    }
private:
    AccessToken          access_token_;
    Entity               entity_;

    std::string          filepath_;     // TODO :: figure out a way to remove this
    std::string          temp_directory_;
    std::string          working_directory_;
    std::string          config_directory_;

    // Shared ptrs,
    FileManager*         file_manager_;
    CredentialsManager*  credentials_manager_;

    TaskDelegate* callback_delegate_;
    std::map<std::string, std::string> config_table_;
};

}//namespace
#endif

