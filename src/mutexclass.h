
#ifndef MUTEXCLASS_H_
#define MUTEXCLASS_H_
#pragma once

#include <iostream>
#include <boost/thread/thread.hpp>

class MutexClass                                                                                  
{                                                                                                 
public:                                                                                           
    MutexClass() {locked = false;}
    virtual ~MutexClass() {}

    void Unlock() {                                                                                             
       m_Mtx.unlock(); 
       locked = false;
    }                                                                                             

    void Lock(int breakcount = -1) {
        try {
            //m_Mtx.lock();

             while(!m_Mtx.try_lock()) {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(1));  
            }
            locked = true;

        }
        catch(boost::lock_error& le) {
            std::cout<<" LOCK ERROR : " << le.what() << std::endl;
        }
    }

private:                                                                                          
    boost::mutex    m_Mtx;
    bool locked; // for debug purposes

};                                                                                                

#endif

