#ifndef MANIFESTTASK_H_
#define MANIFESTTASK_H_
#pragma once

#include "task.h"
#include "filemanager.h"
#include "callbackhandler.h"

namespace attic { 

class ManifestTask : public Task {
public:
    ManifestTask (Task::TaskType type,
                  FileManager* fm,
                  const TaskContext& context)
                  :
                  Task(context, 
                       type) {
        file_manager_ = fm;
    }

    ~ManifestTask() {
        Reset();
    }

    virtual void Reset() {
        file_manager_ = NULL;
    }

    FileManager* file_manager() { return file_manager_; }

protected:
    void Callback(int code, char** pCharArr, int stride, int total) {
        if(context_.delegate()) {
            if(context_.delegate()->type() == TaskDelegate::MANIFEST) {
                ManifestCallback* p = static_cast<ManifestCallback*>(context_.delegate());
                p->Callback(code, pCharArr, stride, total);
            }
        }
    }

private:
    FileManager*         file_manager_;
};

}//namespace
#endif

