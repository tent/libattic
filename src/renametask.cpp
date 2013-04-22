#include "renametask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "utils.h"
#include "event.h"
#include "netlib.h"
#include "taskdelegate.h"
#include "renamestrategy.h"

namespace attic { 

RenameTask::RenameTask(FileManager* pFm, 
                       CredentialsManager* pCm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context,
                       TaskDelegate* callbackDelegate) 
                       :
                       TentTask(Task::RENAME,
                                pFm,
                                pCm,
                                at,
                                entity,
                                context,
                                callbackDelegate)
{
}

RenameTask::~RenameTask() {}

void RenameTask::RunTask() {
    int status = ret::A_OK;

    std::cout<<" Rename task run ... " << std::endl;
    std::string filetype;
    context_.get_value("file_type", filetype);
    std::string file;
    if(filetype == "file") {
        std::string old_file, new_file;
        context_.get_value("original_filepath", old_file);
        context_.get_value("new_filepath", new_file);
        event::RaiseEvent(event::Event::PUSH, event::Event::START, old_file, NULL);
        status = RenameFile(filetype, old_file, new_file);
        event::RaiseEvent(event::Event::PUSH, event::Event::DONE, old_file, NULL);
        file = new_file;
    }
    else if(filetype == "folder") {
        std::string old_folder, new_folder_name;
        context_.get_value("original_folderpath", old_folder);
        context_.get_value("new_foldername", new_folder_name);
        status = RenameFolder(filetype, old_folder, new_folder_name);
        file = new_folder_name;
    }

    Callback(status, file);
    SetFinishedState();
}

int RenameTask::RenameFolder(const std::string& file_type,
                             const std::string& old_folderpath, 
                             const std::string& new_foldername) {
    int status = ret::A_OK;

    HttpStrategyContext rename_context(file_manager(), credentials_manager());
    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

    rename_context.SetConfigValue("file_type", file_type);
    rename_context.SetConfigValue("original_folderpath", old_folderpath);
    rename_context.SetConfigValue("new_foldername", new_foldername);
    rename_context.SetConfigValue("post_path", post_path);
    rename_context.SetConfigValue("posts_feed", posts_feed);
    rename_context.SetConfigValue("entity", entity);

    RenameStrategy rs;
    rename_context.PushBack(&rs);
    status = rename_context.ExecuteAll();


    return status;
}

int RenameTask::RenameFile(const std::string& file_type, 
                           const std::string& old_filepath, 
                           const std::string& new_filepath) {
    int status = ret::A_OK;

    HttpStrategyContext rename_context(file_manager(), credentials_manager());
    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

    rename_context.SetConfigValue("file_type", file_type);
    rename_context.SetConfigValue("post_path", post_path);
    rename_context.SetConfigValue("posts_feed", posts_feed);
    rename_context.SetConfigValue("entity", entity);
    rename_context.SetConfigValue("original_filepath", old_filepath);
    rename_context.SetConfigValue("new_filepath", new_filepath);

    RenameStrategy rs;
    rename_context.PushBack(&rs);
    status = rename_context.ExecuteAll();

    return status;
}

} //namespace
