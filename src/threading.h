
#ifndef THREADING_H_
#define THREADING_H_
#pragma once

#include <iostream>
#include <vector>
#include <boost/thread/thread.hpp>

#include "mutexclass.h"

class ThreadWorker;
class TaskQueue;

class ThreadPool                                                                                 
{                                                                                                
public:                                                                                          
    ThreadPool();
    ~ThreadPool();

    int Initialize();
    int Shutdown();

    int ExtendPool(unsigned int stride);                                                        
    int AbridgePool(unsigned int stride);                                                       

private:                                                                                         
    std::vector<boost::thread*> m_Threads;
    std::vector<ThreadWorker*> m_Workers;

    unsigned int m_ThreadCount;                                                                  
};                                                                                               


// TODO :: remove this, just for testing
volatile static unsigned int g_ThreadCount = 0;

#endif

