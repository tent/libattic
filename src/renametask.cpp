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
    std::string old_file, new_file, folderpath;
    context_.get_value("folderpath", folderpath);
    if(folderpath.empty()) {
        context_.get_value("original_filepath", old_file);
        context_.get_value("new_filepath", new_file);
        event::RaiseEvent(event::Event::PUSH, event::Event::START, old_file, NULL);
        status = RenameFile(old_file, new_file);
        event::RaiseEvent(event::Event::PUSH, event::Event::DONE, old_file, NULL);
    }
    else {
        status = RenameFolder(folderpath);
    }

    Callback(status, new_file);
    SetFinishedState();
}

int RenameTask::RenameFolder(const std::string& folderpath) {
    int status = ret::A_OK;

    HttpStrategyContext rename_context(file_manager(), credentials_manager());
    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

    rename_context.SetConfigValue("folderpath", folderpath);
    rename_context.SetConfigValue("post_path", post_path);
    rename_context.SetConfigValue("posts_feed", posts_feed);
    rename_context.SetConfigValue("entity", entity);

    RenameStrategy rs;
    rename_context.PushBack(&rs);
    status = rename_context.ExecuteAll();


    return status;
}

int RenameTask::RenameFile(const std::string& old_filepath, const std::string& new_filepath) {
    int status = ret::A_OK;

    HttpStrategyContext rename_context(file_manager(), credentials_manager());
    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

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
