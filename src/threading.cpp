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

    std::cout<<" here " << std::endl;
    // Service worker
    ThreadWorker* servicew = new ThreadWorker(file_manager_, 
                                              credentials_manager_, 
                                              access_token_, 
                                              entity_,
                                              true); // Strict
    servicew->SetTaskPreference(Task::SERVICE);
    thread_pool_->SpinOffWorker(servicew);


    std::cout<<" here " << std::endl;
    // Poll worker
    ThreadWorker* pollw = new ThreadWorker(file_manager_, 
                                           credentials_manager_, 
                                           access_token_, 
                                           entity_,
                                           true); // Strict
    std::cout<<" yep " << std::endl;
    pollw->SetTaskPreference(Task::POLL);
    pollw->SetTaskPreference(Task::SERVICE, false);

    std::cout<<" yep " << std::endl;
    thread_pool_->SpinOffWorker(pollw);
    
       // Rename delete
    std::cout<<" here " << std::endl;
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

    std::cout<<" here " << std::endl;
    /*
    ThreadWorker* second_rdw = new ThreadWorker();
    second_rdw->SetTaskPreference(Task::RENAME);
    second_rdw->SetTaskPreference(Task::DELETE);
    second_rdw->SetTaskPreference(Task::POLL, false);
    second_rdw->SetTaskPreference(Task::SERVICE, false);
    workers_.push_back(second_rdw);
    boost::thread* second_thread = new boost::thread(NewThreadFunc, second_rdw);
    threads_.push_back(second_thread);
    */

    std::cout<<" here " << std::endl;
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
    std::cout<<" here " << std::endl;

    /*
    for(unsigned int i=0; i < stride; i++){
        boost::thread* pt = new boost::thread(NewThreadFunc, pWorker);
        threads_.push_back(pt);
        // decide whether or not to keep as a detached thread
        // std::cout<<"detaching thread ... " << std::endl;
        // pt->detach();
    }
    */
    std::cout<<" done " << std::endl;

    return status;
}


}//namespace
