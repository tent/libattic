#ifndef SCANDIRECTORYTASK_H_
#define SCANDIRECTORYTASK_H_
#pragma once

#include "manifesttask.h"

namespace attic { 

class ScanDirectoryTask : public ManifestTask {
    typedef std::vector<std::string> FileVector;
    void CompareToLocalCash(const FileVector& file_list);

public:
    ScanDirectoryTask(FileManager* fm,
                      const TaskContext& context,
                      void (*callback)(int, char**, int, int));

    ~ScanDirectoryTask();

    virtual void OnStart();
    virtual void OnPaused();
    virtual void OnFinished();

    void RunTask();
};

} //namespace
#endif

