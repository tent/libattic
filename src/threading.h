#ifndef THREADING_H_
#define THREADING_H_
#pragma once

#include <iostream>
#include <vector>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"

namespace attic {

class ThreadWorker;
class TaskQueue;

class ThreadPool{
public:                                                                                          
    ThreadPool();
    ~ThreadPool();

    int Initialize();
    int Shutdown();

    int ExtendPool(unsigned int stride);                                                        
    int AbridgePool(unsigned int stride);                                                       
private:                                                                                         
    std::vector<boost::thread*> threads_;
    std::vector<ThreadWorker*> workers_;

    unsigned int thread_count_;                                                                  
};                                                                                               


}//namespace
#endif

