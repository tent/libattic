#ifndef THREADING_H_
#define THREADING_H_
#pragma once

#include <iostream>
#include <vector>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"
#include "filemanager.h"
#include "credentialsmanager.h"
#include "entity.h"
#include "accesstoken.h"

namespace attic {

class ThreadWorker;

class ThreadPool{
public:                                                                                          
    ThreadPool();
    ~ThreadPool();

    int Initialize();
    int Shutdown();

    void SpinOffWorker(ThreadWorker* worker);

    unsigned WorkerCount() { return workers_.size(); }
private:                                                                                         
    std::vector<boost::thread*> threads_;
    std::vector<ThreadWorker*> workers_;

    unsigned int thread_count_;                                                                  
};                                                                                               

class ThreadManager {
    int ExtendPool(unsigned int stride);                                                        
public:
    ThreadManager();
    ~ThreadManager();

    int Initialize(FileManager* fm,
                   CredentialsManager* cm,
                   const AccessToken& at,
                   const Entity ent,
                   unsigned int poolSize);



    int Shutdown();

private:
    MutexClass  pool_mtx_; // This may not even be needed, is where is the possible contention?
    ThreadPool* thread_pool_;

    FileManager*            file_manager_;
    CredentialsManager*     credentials_manager_;
    AccessToken             access_token_;
    Entity                  entity_;
};


}//namespace
#endif

