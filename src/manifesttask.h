#ifndef MANIFESTTASK_H_
#define MANIFESTTASK_H_
#pragma once

#include "task.h"
#include "filemanager.h"

namespace attic { 

class ManifestTask : public Task {
public:
    ManifestTask (Task::TaskType type,
                  FileManager* fm,
                  void (*callback)(int, char**, int, int))
                  :
                  Task(type) {
        file_manager_ = fm;
        callback_ = callback;
    }

    ~ManifestTask() {
        Reset();
    }

    virtual void Reset() {
        file_manager_ = NULL;
        callback_ = NULL;
    }

    FileManager* file_manager() { return file_manager_; }

protected:
    void Callback(int code, char** pCharArr, int stride, int total) {
        if(callback_)
            callback_(code, pCharArr, stride, total);
    }

private:
    FileManager*         file_manager_;

    void(*callback_)(int, char**, int, int);
};

}//namespace
#endif

