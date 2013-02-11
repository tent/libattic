#include "queryfilestask.h"
#include "errorcodes.h"


QueryFilesTask::QueryFilesTask( Task::TaskType type,                    
                                FileManager* pFm,                       
                                void (*callback)(int, char**, int, int))
                                :
                                ManifestTask( type,
                                              pFm,
                                              callback)
{

}

QueryFilesTask::~QueryFilesTask()
{

}

void QueryFilesTask::RunTask()
{

}

int QueryManifest()
{
    int status = ret::A_OK;

    return status;
}


