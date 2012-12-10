

#ifndef TASK_H_
#define TASK_H_
#pragma once

class Task // Inherit from this to implement specific tasks   
{                                                             
public:                                                       
    Task(){}                                                  
    virtual ~Task(){}                                         
    
    virtual void RunTask() = 0;                               
};                                                            


#endif

