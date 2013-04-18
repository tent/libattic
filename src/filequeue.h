#ifndef FILEQUEUE_H_
#define FILEQUEUE_H_
#pragma once

#include <map>
#include <string>

#include "mutexclass.h"
#include "event.h"

namespace attic { 

class FileQueue {
public:
    FileQueue() {}
    ~FileQueue() {}

    virtual bool LockFile(const std::string& filepath) {
        if(!file_queue_[filepath]) {
            file_queue_[filepath] = true;
            return true;
        }
        return false;
    }

    virtual bool UnlockFile(const std::string& filepath) {
        if(file_queue_[filepath]) {
            file_queue_[filepath] = false;
            return true;
        }
        return false;
    }
    
    bool IsFileLocked(const std::string& filepath) {
        if(file_queue_.find(filepath) != file_queue_.end())
            return file_queue_[filepath];
        return false;
    }

private:
    // filepath, locked
    std::map<std::string, bool> file_queue_;
};

class CentralFileQueue : public MutexClass {

    CentralFileQueue(const CentralFileQueue& rhs) {}
    CentralFileQueue operator=(const CentralFileQueue& rhs) { return *this; }
public:
    CentralFileQueue() {}
    ~CentralFileQueue() {}

    bool LockFile(const std::string& filepath) {
        bool retval = false;
        Lock();
        retval = file_queue_.LockFile(filepath);
        event::RaiseEvent(event::Event::FILE_LOCK, filepath, NULL);
        Unlock();
        return retval;
    }

    bool UnlockFile(const std::string& filepath) {
        bool retval = false;
        Lock();
        retval = file_queue_.UnlockFile(filepath);
        event::RaiseEvent(event::Event::FILE_UNLOCK, filepath, NULL);
        Unlock();
        return retval;
    }

    bool IsFileLocked(const std::string& filepath) {
        bool retval = false;
        Lock();
        retval = file_queue_.IsFileLocked(filepath);
        Unlock();
        return retval;
    }

    static CentralFileQueue* GetInstance() {
        if(!instance_)
            instance_ = new CentralFileQueue();
        return instance_;
    }

private:
    FileQueue file_queue_;
    static CentralFileQueue* instance_;
};

}//namespace
#endif


