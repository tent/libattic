#ifndef QUERYFILESTASK_H_
#define QUERYFILESTASK_H_
#pragma once

#include "manifesttask.h"

class QueryFilesTask : public ManifestTask
{
public:
    QueryFilesTask( Task::TaskType type,                    
                  FileManager* pFm,                       
                  void (*callback)(int, char**, int, int));

    ~QueryFilesTask();

    virtual void OnStart() { } 
    virtual void OnPaused() { } 
    virtual void OnFinished() { }

    void RunTask();

private:


};

#endif

