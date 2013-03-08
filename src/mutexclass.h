
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

    void Unlock()                                                                                  
    {                                                                                             
       if(!locked)
           std::cout<<"UNLOCKING MUTEX THATS ALREADY UNLOCKED!"<<std::endl;

       m_Mtx.unlock(); 
       locked = false;
    }                                                                                             

    void Lock(int breakcount = -1) 
    {
        if(locked)
            std::cout<<" TRYING TO DOUBLE LOCK " << std::endl;

        try {
            //m_Mtx.lock();

             while(!m_Mtx.try_lock()) {
                std::cout<<"trying..."<<std::endl;
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

