#ifndef RESPONSE_H_
#define RESPONSE_H_
#pragma once

#include <string>

struct Response                              
{                                            
    int code =0;                             
    std::string body;                        

    std::string CodeAsString()               
    {                                        
        char buf[256]={'\0'};                
        sprintf(buf,"%d", code);             
        std::string codestr;                 
        codestr.append(buf);                 
        return codestr;                      
    }                                        

    void clear()                             
    {                                        
        code = -1;                           
        body.clear();                        
    }                                        
};                                           


#endif

