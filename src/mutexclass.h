
#ifndef MUTEXCLASS_H_
#define MUTEXCLASS_H_
#pragma once

#include <unistd.h>
#include <pthread.h>

class MutexClass                                                                                  
{                                                                                                 
public:                                                                                           
    MutexClass()                                                                                  
    {                                                                                             
        pthread_mutex_init(&m_Mutex, NULL);                                                        
    }                                                                                             

    virtual ~MutexClass()                                                                         
    {                                                                                             
        pthread_mutex_destroy(&m_Mutex);                                                          
    }                                                                                             

    int TryLock()                                                                                 
    {                                                                                             
        return pthread_mutex_trylock(&m_Mutex);        
    } // as all things unix 0 is ok                                                               

    int Unlock()                                                                                  
    {                                                                                             
        return pthread_mutex_unlock(&m_Mutex);      
    }                                                                                             

//protected:
    int Lock(int breakcount = -1)
    {
        int count = 0;
        while(TryLock())
        { 
            sleep(0); 
            if(breakcount > -1)
            {
                if(count > breakcount)
                    return 1;
                count++;
            }
        }  
        return 0;
    }

private:                                                                                          
    pthread_mutex_t     m_Mutex;                                                                      

};                                                                                                

#endif

