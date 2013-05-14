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
                       const TaskContext& context)
                       :
                       TentTask(Task::RENAME,
                                pFm,
                                pCm,
                                at,
                                entity,
                                context)
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
        std::string old_filepath, new_filepath;
        context_.get_value("original_filepath", old_filepath);
        context_.get_value("new_filepath", new_filepath);

        event::RaiseEvent(event::Event::RENAME, event::Event::START, old_filepath, NULL);
        status = RenameFile(filetype, old_filepath, new_filepath);
        event::RaiseEvent(event::Event::RENAME, event::Event::DONE, old_filepath, NULL);

        file = new_filepath;
    }
    else if(filetype == "folder") {
        std::string old_folderpath, new_folderpath;
        context_.get_value("original_folderpath", old_folderpath);
        context_.get_value("new_folderpath", new_folderpath);

        event::RaiseEvent(event::Event::RENAME, event::Event::START, old_folderpath, NULL);
        status = RenameFolder(filetype, old_folderpath, new_folderpath);
        event::RaiseEvent(event::Event::RENAME, event::Event::DONE, old_folderpath, NULL);

        file = new_folderpath;
    }

    Callback(status, file);
    SetFinishedState();
}

int RenameTask::RenameFolder(const std::string& file_type,
                             const std::string& old_folderpath, 
                             const std::string& new_folderpath) {
    int status = ret::A_OK;

    HttpStrategyContext rename_context(file_manager(), credentials_manager());
    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

    rename_context.SetConfigValue("file_type", file_type);
    rename_context.SetConfigValue("original_folderpath", old_folderpath);
    rename_context.SetConfigValue("new_folderpath", new_folderpath);
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

    std::cout<<" filepath : " << old_filepath << std::endl;

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
