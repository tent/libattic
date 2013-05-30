#include "uploadtask.h"

#include "filehandler.h"
#include "postfilestrategy.h"

namespace attic { 

UploadTask::UploadTask(FileManager* fm,
                       CredentialsManager* cm,
                       const AccessToken& at,
                       const Entity& entity,
                       const TaskContext& context) 
                       :
                       TentTask(Task::UPLOADFILE,
                                fm,
                                cm,
                                at,
                                entity,
                                context)
{
}

UploadTask::~UploadTask() {}

void UploadTask::RunTask() {
    int status = ret::A_OK;
    std::string post_id;
    context_.get_value("post_id", post_id);
    if(!post_id.empty()) { 
        FileInfo fi;
        status = RetrieveFileInfo(post_id, fi);
        if(status == ret::A_OK) {
            std::string filepath;
            file_manager()->GetCanonicalFilepath(fi.filepath(), filepath);
            if(!file_manager()->IsFileLocked(filepath)) {
                file_manager()->LockFile(filepath);
                event::RaiseEvent(event::Event::PUSH, event::Event::START, filepath, NULL);
                status = ProcessFile(fi);
                event::RaiseEvent(event::Event::PUSH, event::Event::DONE, filepath, NULL);
                file_manager()->UnlockFile(filepath);
            }
            else {
                std::string error = " File is locked by other task, finishing task\n";
                error += " file : " + filepath + "\n";
                log::LogString("333MA!941", error);
                status = ret::A_FAIL_FILE_IN_USE;
            }
        }
    }
    else {
        status = ret::A_FAIL_INVALID_POST_ID;
    }

    Callback(status, post_id);
    SetFinishedState();
}

int UploadTask::RetrieveFileInfo(const std::string& post_id, FileInfo& out) {
    int status = ret::A_OK;
    FileHandler fh(file_manager());
    if(!fh.RetrieveFileInfoById(post_id, out))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    return status;
}

int UploadTask::ProcessFile(const FileInfo& fi) {
    int status = ret::A_OK;

    HttpStrategyContext pushcontext(file_manager(), credentials_manager());
    PostFileStrategy ps;

    std::string post_path = GetPostPath();
    std::string posts_feed = TentTask::entity().GetPreferredServer().posts_feed();
    std::string entity = TentTask::entity().entity();

    std::string filepath;
    FileHandler fh(file_manager());
    fh.GetCanonicalFilepath(fi.filepath(), filepath);

    pushcontext.SetConfigValue("post_path", post_path);
    pushcontext.SetConfigValue("posts_feed", posts_feed);
    pushcontext.SetConfigValue("filepath", filepath);
    pushcontext.SetConfigValue("entity", entity);
    pushcontext.SetConfigValue("post_id", fi.post_id());
    pushcontext.PushBack(&ps);
    status = pushcontext.ExecuteAll();
    return status;
}

} // namespace

