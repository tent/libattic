#ifndef RESPONSE_H_
#define RESPONSE_H_
#pragma once

#include <string>

#include "httpheader.h"

class Response                              
{                                            
public:
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

    void ConsumeHeader(const std::string& in) 
    {
        header.ParseString(in);
    }

    int code =0;                             
    std::string body;                        
    HttpHeader header;  // Response Header
};                                           


#endif

