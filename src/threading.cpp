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
    for(unsigned int i=0; i<m_Workers.size(); i++) {
        m_Workers[i]->SetThreadExit();
    }

    std::cout<<" joining threads ... " << std::endl;
    for(unsigned int i=0; i<m_Threads.size(); i++) {
        try {
            std::cout<<" joining : " << i << std::endl;
            std::cout<<" joinable : " << m_Threads[i]->joinable() << std::endl;
            if(m_Threads[i]->joinable()) {
                m_Threads[i]->join();
            }
        }
        catch(boost::system::system_error& ti) {
            std::cout<<" join error : " << ti.what() << std::endl;
        }
    }

    std::cout<<" done " << std::endl;
    return ret::A_OK;
}

int ThreadPool::ExtendPool(unsigned int stride) {
    std::cout<<" extending thread pool " << std::endl;
    int status = ret::A_OK;

    for(unsigned int i=0; i < stride; i++){
        ThreadWorker* pWorker = new ThreadWorker();
        m_Workers.push_back(pWorker);

        boost::thread* pt = new boost::thread(NewThreadFunc, pWorker);
        m_Threads.push_back(pt);

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
        m_ThreadCount--;
    }
    */

    return status;
}

}//namespace
