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

#include "filemanager.h"
#include "credentialsmanager.h"
#include "taskarbiter.h"
#include "tentapp.h"
#include "scandirectorytask.h"
#include "logutils.h"

namespace attic { 

TaskFactory::TaskFactory() {}
TaskFactory::~TaskFactory() {}

Task* TaskFactory::GetTentTask(int type,
                               FileManager* pFm,             
                               CredentialsManager* pCm,      
                               const AccessToken& at,        
                               const Entity& entity,    
                               const TaskContext& context) {
    Task* t = NULL;
    t = CreateNewTentTask(type,
                          pFm,       
                          pCm,
                          at,
                          entity,    
                          context);

    return t;
}


Task* TaskFactory::GetManifestTask(Task::TaskType type,
                                   FileManager* pFm,
                                   const TaskContext& context,
                                   void (*callback)(int, char**, int, int)) {
    Task* t = NULL;
    t = CreateNewManifestTask(type,
                              pFm,
                              context,
                              callback);
    return t;
}

Task* TaskFactory::CreateNewManifestTask(Task::TaskType type,
                                         FileManager* pFm,
                                         const TaskContext& context,
                                         void (*callback)(int, char**, int, int)) {
    Task* t = NULL;
    switch(type) {
        case Task::QUERYMANIFEST:
        {
            t = new QueryFilesTask(pFm, context, callback);
            break;
        }
        case Task::SCANDIRECTORY:
        {
            t = new ScanDirectoryTask(pFm, context, callback);
            break;
        }
        default:
        {
            LogUnknownTaskType(type);
            break;
        }
    }

    return t;
}

Task* TaskFactory::CreateNewTentTask(int type,                  
                                     FileManager* pFm,               
                                     CredentialsManager* pCm,        
                                     const AccessToken& at,          
                                     const Entity& entity,      
                                     const TaskContext& context) {

    Task* t = NULL;
    switch(type)
    {
        case Task::PUSH:
        {
            t = new PushTask(pFm,
                             pCm,                    
                             at,
                             entity,                          
                             context);
            break;
        }
        case Task::PULL:
        {
            t = new PullTask(pFm,                     
                             pCm,          
                             at,
                             entity,                           
                             context);
            break;
        }
        case Task::RENAME:
        {
            t = new RenameTask(pFm,
                               pCm,
                               at,
                               entity,
                               context);
            break;
        }
        case Task::DELETE:
        {
            t = new DeleteTask(pFm,                          
                               pCm,          
                               at,
                               entity,                                
                               context);
            break;
        }
        case Task::SYNC_FILE_TASK:
        {
            t = new SyncFileTask(pFm,
                                 pCm,
                                 at,
                                 entity,
                                 context);

            break;
        }
        case Task::POLL:
        {
            t = new PollTask(pFm,                   
                             pCm,                   
                             at,                    
                             entity,                
                             context);
            break;
        }
        default:
        {
            std::cout<<" CREATING UNKNOWN TASK " << std::endl;
        }
    }


    return t;
}

void TaskFactory::LogUnknownTaskType(Task::TaskType type) {
    char buf[256] = {'\0'};
    sprintf(buf, "%d", type);
    std::string a = "Unknown task type : ";
    a.append(buf);
    log::LogString("KJASD123", a);
}

}//namespace

