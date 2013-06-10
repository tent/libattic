#include "threading.h"

#include <iostream>
#include <deque>
#include <unistd.h>

#include "errorcodes.h"
#include "task.h"
#include "taskqueue.h"
#include "threadworker.h"

namespace attic {

void NewThreadFunc(ThreadWorker *pWorker) {
    std::cout<<" thread starting " << std::endl;
    if(pWorker) {
        pWorker->Run();
        delete pWorker;
        pWorker = NULL;
    }
    std::cout<<" thread ending " << std::endl;
}

ThreadPool::ThreadPool() {}                                               
ThreadPool::~ThreadPool() {}

int ThreadPool::Initialize() {
    return ret::A_OK;
}

int ThreadPool::Shutdown() {
    std::cout<<" shutting down pool " << std::endl;
    for(unsigned int i=0; i<workers_.size(); i++) {
        workers_[i]->SetThreadExit();
        //workers_[i]->SetThreadShutdown();
    }

    std::cout<<" joining threads ... " << std::endl;
    for(unsigned int i=0; i<threads_.size(); i++) {
        try {
            std::cout<<" joining : " << i << std::endl;
            std::cout<<" joinable : " << threads_[i]->joinable() << std::endl;
            if(threads_[i]->joinable()) {
                threads_[i]->join();
                delete threads_[i];
                threads_[i] = NULL;
            }
        }
        catch(boost::system::system_error& ti) {
            std::cout<<" join error : " << ti.what() << std::endl;
        }
    }

    std::cout<<" done shutting down pool " << std::endl;
    return ret::A_OK;
}

void ThreadPool::SpinOffWorker(ThreadWorker* worker) {
    if(worker) {
        try {
            workers_.push_back(worker);
            boost::thread* thread = new boost::thread(NewThreadFunc, worker);
            threads_.push_back(thread);
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

    // Dedicated Push
    ThreadWorker* pushw = new ThreadWorker(file_manager_, 
                                             credentials_manager_, 
                                             access_token_, 
                                             entity_, 
                                             true); // Strict
    pushw->SetTaskPreference(Task::PUSH);
    thread_pool_->SpinOffWorker(pushw);
    // Dedicated Upload
    for(int i=0; i<4; i++) {
       ThreadWorker* upw = new ThreadWorker(file_manager_, 
                                            credentials_manager_, 
                                            access_token_, 
                                            entity_, 
                                            true); // Strict
        pushw->SetTaskPreference(Task::UPLOADFILE);
        thread_pool_->SpinOffWorker(upw);
 
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
