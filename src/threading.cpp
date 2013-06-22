#include "threading.h"

#include <iostream>
#include <deque>
#include <unistd.h>

#include "errorcodes.h"
#include "task.h"
#include "taskqueue.h"
#include "threadworker.h"

namespace attic {

ThreadPool::ThreadPool() {}                                               
ThreadPool::~ThreadPool() {}

int ThreadPool::Initialize() {
    return ret::A_OK;
}

int ThreadPool::Shutdown() {
    std::cout<<" shutting down pool " << std::endl;
    for(unsigned int i=0; i<workers_.size(); i++) {
        workers_[i]->StopThread();
        delete workers_[i];
        workers_[i] = NULL;
    }
    std::cout<<" done shutting down pool " << std::endl;
    return ret::A_OK;
}

void ThreadPool::SpinOffWorker(ThreadWorker* worker) {
    if(worker) {
        try {
            workers_.push_back(worker);
            worker->StartThread();
        }
        catch(std::exception& e) {
            std::cout<<" SPIN OFF WORKER EXCEPTION : " << e.what() << std::endl;
        }
    }
}


ThreadManager::ThreadManager(FileManager* fm,
                             CredentialsManager* cm,
                             const AccessToken& at,
                             const Entity& ent){
    thread_pool_ = NULL;
    file_manager_ = fm;
    credentials_manager_ = cm;
    access_token_ = at;
    entity_ = ent;
}

ThreadManager::~ThreadManager() {}

int ThreadManager::Initialize(unsigned int poolSize) {
    int status = ret::A_OK;

    if(!thread_pool_) {
        thread_pool_ = new ThreadPool();
    }
    pool_mtx_.Lock();
    if(thread_pool_) {
        thread_pool_->Initialize();
        status = ExtendPool(poolSize);
    }
    else {
        status = ret::A_FAIL_INVALID_PTR;
    }
    pool_mtx_.Unlock();
    return status; 
}

int ThreadManager::Shutdown() {
    int status = ret::A_OK;
    pool_mtx_.Lock();
    if(thread_pool_) {
        status = thread_pool_->Shutdown();
        delete thread_pool_;
        thread_pool_ = NULL;
    }
    pool_mtx_.Unlock();

    file_manager_ = NULL;
    credentials_manager_ = NULL;
    return status;
}

int ThreadManager::ExtendPool(unsigned int stride) {
    std::cout<<" extending thread pool " << std::endl;
    int status = ret::A_OK;

    // Service worker
    ThreadWorker* servicew = new ThreadWorker(file_manager_, 
                                              credentials_manager_, 
                                              access_token_, 
                                              entity_,
                                              true); // Strict
    servicew->SetTaskPreference(Task::SERVICE);
    thread_pool_->SpinOffWorker(servicew);

    // Poll worker
    ThreadWorker* pollw = new ThreadWorker(file_manager_, 
                                           credentials_manager_, 
                                           access_token_, 
                                           entity_,
                                           true); // Strict
    pollw->SetTaskPreference(Task::POLL);
    pollw->SetTaskPreference(Task::SERVICE, false);
    thread_pool_->SpinOffWorker(pollw);
    
    // Rename delete
    ThreadWorker* rdw = new ThreadWorker(file_manager_, 
                                         credentials_manager_, 
                                         access_token_, 
                                         entity_, 
                                         true); // Strict
    rdw->SetTaskPreference(Task::RENAME);
    rdw->SetTaskPreference(Task::DELETE);
    rdw->SetTaskPreference(Task::POLL, false);
    rdw->SetTaskPreference(Task::SERVICE, false);
    thread_pool_->SpinOffWorker(rdw);

    // Folder Worker
    ThreadWorker* folderw = new ThreadWorker(file_manager_, 
                                             credentials_manager_, 
                                             access_token_, 
                                             entity_, 
                                             true); // Strict
    folderw->SetTaskPreference(Task::FOLDER);
    thread_pool_->SpinOffWorker(folderw);

    // Folder Worker
    ThreadWorker* folderw2 = new ThreadWorker(file_manager_, 
                                             credentials_manager_, 
                                             access_token_, 
                                             entity_, 
                                             true); // Strict
    folderw2->SetTaskPreference(Task::FOLDER);
    thread_pool_->SpinOffWorker(folderw2);

    // Dedicated push
    ThreadWorker* pushw = new ThreadWorker(file_manager_, 
                                           credentials_manager_, 
                                           access_token_, 
                                           entity_, 
                                           true); // Strict
    pushw->SetTaskPreference(Task::PUSH);
    thread_pool_->SpinOffWorker(pushw);

    for(int i=0;i<2;i++) {
        ThreadWorker* uw = new ThreadWorker(file_manager_, 
                                            credentials_manager_, 
                                            access_token_, 
                                            entity_, 
                                            true); // Strict
        uw->SetTaskPreference(Task::UPLOADFILE);
        thread_pool_->SpinOffWorker(uw);
    }

    // Dedicated download
    ThreadWorker* pullw = new ThreadWorker(file_manager_, 
                                           credentials_manager_, 
                                           access_token_, 
                                           entity_, 
                                           true); // Strict
    pullw->SetTaskPreference(Task::PULL);
    thread_pool_->SpinOffWorker(pullw);


    // Generic (Push/Pull) workers
    for(unsigned int i=0; i < stride; i++){
        ThreadWorker* pWorker = new ThreadWorker(file_manager_, credentials_manager_, access_token_, entity_);
        pWorker->SetTaskPreference(Task::POLL, false);
        pWorker->SetTaskPreference(Task::SERVICE, false);
        pWorker->SetTaskPreference(Task::RENAME, false);
        pWorker->SetTaskPreference(Task::DELETE, false);

        thread_pool_->SpinOffWorker(pWorker);
        if(thread_pool_->WorkerCount() >= stride)
            break;
    }

    return status;
}


}//namespace
