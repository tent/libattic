#ifndef SERVICEMANAGER_H_
#define SERVICEMANAGER_H_
#pragma once

#include "taskmanager.h"

namespace attic { 

class ServiceManager {
    int StartupServiceThread();
public:
    ServiceManager();
    ~ServiceManager();

    int Initialize();
    int Shutdown();

    TaskManager* task_manager() const { return task_manager_; }
    void set_task_manager(TaskManager* tm) { task_manager_ = tm; }

private:
    TaskManager* task_manager_;
};

} //namespace
#endif

