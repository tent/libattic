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

    for(unsigned int i=0; i < stride; i++){
        ThreadWorker* pWorker = new ThreadWorker();
        workers_.push_back(pWorker);

        boost::thread* pt = new boost::thread(NewThreadFunc, pWorker);
        threads_.push_back(pt);

        // decide whether or not to keep as a detached thread
//        std::cout<<"detaching thread ... " << std::endl;
//        pt->detach();

    }

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
