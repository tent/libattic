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

int ThreadPool::ExtendPool(unsigned int stride) {
    std::cout<<" extending thread pool " << std::endl;
    int status = ret::A_OK;

    // Service worker
    ThreadWorker* servicew = new ThreadWorker(true); // Strict
    servicew->SetTaskPreference(Task::SERVICE);
    workers_.push_back(servicew);
    boost::thread* service_thread = new boost::thread(NewThreadFunc, servicew);
    threads_.push_back(service_thread);


    // Poll worker
    ThreadWorker* pollw = new ThreadWorker(true); // Strict
    pollw->SetTaskPreference(Task::POLL);
    pollw->SetTaskPreference(Task::SERVICE, false);
    workers_.push_back(pollw);
    boost::thread* poll_thread = new boost::thread(NewThreadFunc, pollw);
    threads_.push_back(poll_thread);

    // Rename delete
    ThreadWorker* rdw = new ThreadWorker();
    rdw->SetTaskPreference(Task::RENAME);
    rdw->SetTaskPreference(Task::DELETE);
    rdw->SetTaskPreference(Task::POLL, false);
    rdw->SetTaskPreference(Task::SERVICE, false);
    workers_.push_back(rdw);
    boost::thread* rdw_thread = new boost::thread(NewThreadFunc, rdw);
    threads_.push_back(rdw_thread);

    ThreadWorker* second_rdw = new ThreadWorker();
    second_rdw->SetTaskPreference(Task::RENAME);
    second_rdw->SetTaskPreference(Task::DELETE);
    second_rdw->SetTaskPreference(Task::POLL, false);
    second_rdw->SetTaskPreference(Task::SERVICE, false);
    workers_.push_back(second_rdw);
    boost::thread* second_thread = new boost::thread(NewThreadFunc, second_rdw);
    threads_.push_back(second_thread);

    // Generic workers
    for(unsigned int i=0; i < stride; i++){
        ThreadWorker* pWorker = new ThreadWorker();
        pWorker->SetTaskPreference(Task::POLL, false);
        pWorker->SetTaskPreference(Task::SERVICE, false);
        workers_.push_back(pWorker);

        boost::thread* pt = new boost::thread(NewThreadFunc, pWorker);
        threads_.push_back(pt);
        if(workers_.size() >= stride)
            break;
    }

    /*
    for(unsigned int i=0; i < stride; i++){
        boost::thread* pt = new boost::thread(NewThreadFunc, pWorker);
        threads_.push_back(pt);
        // decide whether or not to keep as a detached thread
        // std::cout<<"detaching thread ... " << std::endl;
        // pt->detach();
    }
    */

    return status;
}

int ThreadPool::AbridgePool(unsigned int stride) {
    int status = ret::A_OK;
    std::cout<<" not implemented " <<std::endl;
    /*
    for(unsigned int i=0; i < stride; i++)
    {
        while(m_ThreadData[i]->State.TryLock()) { sleep(0); }
        m_ThreadData[i]->State.SetStateExit();
        m_ThreadData[i]->State.Unlock();
        thread_count_--;
    }
    */

    return status;
}

}//namespace
