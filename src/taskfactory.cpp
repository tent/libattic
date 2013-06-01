#include "taskfactory.h"

#include "tenttask.h"
#include "pulltask.h"
#include "pushtask.h"
#include "deletetask.h"
#include "queryfilestask.h"
#include "polltask.h"
#include "syncfiletask.h"
#include "renametask.h"
#include "servicetask.h"
#include "foldertask.h"
#include "uploadtask.h"
#include "metatask.h"

#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskarbiter.h"
#include "tentapp.h"
#include "scandirectorytask.h"
#include "logutils.h"

namespace attic { 

TaskFactory::TaskFactory() {
    file_manager_ = NULL;
    credentials_manager_ = NULL;
}

TaskFactory::~TaskFactory() {}

void TaskFactory::Initialize(FileManager* fm,             
                             CredentialsManager* cm,      
                             const AccessToken& at,        
                             const Entity& ent) {
    file_manager_ = fm;
    credentials_manager_ = cm;
    access_token_ = at;
    entity_ = ent;
}

void TaskFactory::Shutdown() {}

Task* TaskFactory::GetTentTask(const TaskContext& context) {
    Task* t = NULL;
    t = CreateNewTentTask(context);
    return t;
}


Task* TaskFactory::GetManifestTask(Task::TaskType type,
                                   FileManager* pFm,
                                   const TaskContext& context,
                                   void (*callback)(int, char**, int, int)) {
    Task* t = NULL;
    t = CreateNewManifestTask(context,
                              callback);
    return t;
}

void TaskFactory::ReclaimTask(Task* task) {
    if(task) {
        delete task;
        task = NULL;
    }
}

Task* TaskFactory::CreateNewManifestTask(const TaskContext& context,
                                         void (*callback)(int, char**, int, int)) {
    Task* t = NULL;
    switch(context.type()) {
        case Task::QUERYMANIFEST:
        {
            t = new QueryFilesTask(file_manager_, context);
            break;
        }
        case Task::SCANDIRECTORY:
        {
            t = new ScanDirectoryTask(file_manager_, context);
            break;
        }
        default:
        {
            LogUnknownTaskType(context.type());
            break;
        }
    }

    return t;
}

Task* TaskFactory::CreateNewTentTask(const TaskContext& context) {
    Task* t = NULL;
    switch(context.type())
    {
        case Task::PUSH:
        {
            t = new PushTask(file_manager_,
                             credentials_manager_,                    
                             access_token_,
                             entity_,                          
                             context);
            break;
        }
        case Task::PULL:
        {
            t = new PullTask(file_manager_,
                             credentials_manager_,
                             access_token_,
                             entity_,
                             context);
            break;
        }
        case Task::RENAME:
        {
            t = new RenameTask(file_manager_,
                               credentials_manager_,
                               access_token_,
                               entity_,
                               context);
            break;
        }
        case Task::DELETE:
        {
            t = new DeleteTask(file_manager_,
                               credentials_manager_,
                               access_token_,
                               entity_,
                               context);
            break;
        }
        case Task::SYNC_FILE_TASK:
        {
            t = new SyncFileTask(file_manager_,
                                 credentials_manager_,
                                 access_token_,
                                 entity_,
                                 context);
            break;
        }
        case Task::POLL:
        {
            t = new PollTask(file_manager_,
                             credentials_manager_,
                             access_token_,
                             entity_,
                             context);
            break;
        }
        case Task::SERVICE:
        {
            t = new ServiceTask(file_manager_,
                                credentials_manager_,
                                access_token_,
                                entity_,
                                context);
            break;
        }
        case Task::FOLDER:
        {
            t = new FolderTask(file_manager_,
                               credentials_manager_,
                               access_token_,
                               entity_,
                               context);
            break;
        }
        case Task::UPLOADFILE:
        {
            t = new UploadTask(file_manager_,
                               credentials_manager_,
                               access_token_,
                               entity_,
                               context);
            break;
        }
        case Task::META:
        {
            t = new MetaTask(file_manager_,
                             credentials_manager_,
                             access_token_,
                             entity_,
                             context);
            break;
        }
        default:
        {
            std::cout<<" CREATING UNKNOWN TASK " << std::endl;
            std::cout<< " TASK TYPE : " << context.type() << std::endl;
        }
    }
    return t;
}

void TaskFactory::LogUnknownTaskType(int type) {
    char buf[256] = {'\0'};
    sprintf(buf, "%d", type);
    std::string a = "Unknown task type : ";
    a.append(buf);
    log::LogString("KJASD123", a);
}

}//namespace

