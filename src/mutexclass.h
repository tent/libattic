
#ifndef MUTEXCLASS_H_
#define MUTEXCLASS_H_
#pragma once

#include <iostream>
#include <boost/thread/thread.hpp>

class MutexClass                                                                                  
{                                                                                                 
public:                                                                                           
    MutexClass() {}
    virtual ~MutexClass() {}

    void Unlock()                                                                                  
    {                                                                                             
       std::cout<<" unlock " << std::endl;
       m_Mtx.unlock(); 
    }                                                                                             

    void Lock(int breakcount = -1) 
    {
        std::cout<<" lock " << std::endl;
        while(!m_Mtx.try_lock()) {
            std::cout<<"trying..."<<std::endl;
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1));  
        }
    }

private:                                                                                          
    boost::mutex    m_Mtx;

};                                                                                                

#endif

