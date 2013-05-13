#ifndef SERVICEMANAGER_H_
#define SERVICEMANAGER_H_
#pragma once

#include "taskmanager.h"

namespace attic { 

class ServiceManager {
    int StartupServiceThread();
public:
    ServiceManager(TaskManager* tm);
    ~ServiceManager();

    int Initialize();
    int Shutdown();

    TaskManager* task_manager() const { return task_manager_; }
private:
    TaskManager* task_manager_;
};

} //namespace
#endif

