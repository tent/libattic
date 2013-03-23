#include "pushtask.h"

#include "filesystem.h"
#include "filemanager.h"
#include "errorcodes.h"
#include "event.h"
#include "taskdelegate.h"

#include "postfilestrategy.h"
#include "postfilemetadatastrategy.h"
#include "postfoldermetadatastrategy.h"

PushTask::PushTask(FileManager* pFm, 
                   CredentialsManager* pCm,
                   const AccessToken& at,
                   const Entity& entity,
                   const std::string& filepath,
                   const std::string& tempdir,
                   const std::string& workingdir,
                   const std::string& configdir,
                   TaskDelegate* callbackDelegate)
                   :
                   TentTask (Task::PUSH,
                             pFm,
                             pCm,
                             at,
                             entity,
                             filepath,
                             tempdir,
                             workingdir,
                             configdir,
                             callbackDelegate)
{
}

PushTask::~PushTask() {}

void PushTask::RunTask() {
    // Run the task
    std::string filepath;
    GetFilepath(filepath);

    event::RaiseEvent(event::Event::PUSH, event::Event::START, filepath, NULL);
    int status = PushFile(filepath);
    event::RaiseEvent(event::Event::PUSH, event::Event::DONE, filepath, NULL);
    
    // Callback
    Callback(status, filepath);
    SetFinishedState();
}

// Note* path should not be relative, let the filemanager take care of
// all the canonical to relative path conversions
int PushTask::PushFile(const std::string& filepath) {
    int status = ret::A_OK;

    if(fs::CheckFileExists(filepath)) {
        std::string apiroot;
        GetApiRoot(apiroot);
        Response resp;

        PostFileStrategy ps;                // Chunk and upload
        PostFileMetadataStrategy pmds;      // Update file post
        PostFolderMetadataStrategy pfmds;   // Update folder post
        if(!GetFileManager()) std::cout<<" Invalid File Manager " << std::endl;
        if(!GetCredentialsManager()) std::cout<<" Invalid Cred Manager " << std::endl;

        HttpStrategyContext pushcontext(GetFileManager(), 
                                        GetCredentialsManager());
        pushcontext.SetConfigValue("api_root", apiroot);
        pushcontext.SetConfigValue("filepath", filepath);

        pushcontext.PushBack(&ps);
        pushcontext.PushBack(&pmds);
        pushcontext.PushBack(&pfmds);

        status = pushcontext.ExecuteAll();
        
    }
    else {
        status = ret::A_FAIL_OPEN_FILE;
    }

    return status;
}

