#ifndef MANIFESTTASK_H_
#define MANIFESTTASK_H_
#pragma once

#include "task.h"
#include "filemanager.h"

namespace attic { 

class ManifestTask : public Task
{
public:
    ManifestTask ( Task::TaskType type,
                   FileManager* pFm,
                   void (*callback)(int, char**, int, int))
                   :
                   Task(type)
    {
        m_pFileManager = pFm;
        mCallback = callback;
    }

    ~ManifestTask()
    {
        Reset();
    }

    virtual void Reset()
    {
        m_pFileManager = NULL;
        mCallback = NULL;
    }

    FileManager* GetFileManager() { return m_pFileManager; }

protected:
    void Callback(int code, char** pCharArr, int stride, int total)
    {
        if(mCallback)
            mCallback(code, pCharArr, stride, total);
    }

private:
    FileManager*         m_pFileManager;

    void(*mCallback)(int, char**, int, int);
};

}//namespace
#endif

