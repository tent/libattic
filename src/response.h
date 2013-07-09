#ifndef RESPONSE_H_
#define RESPONSE_H_
#pragma once

#include <string>
#include <stdio.h>
#include "httpheader.h"

namespace attic { 

class Response {
public:
    Response() {
        code = 0;
    }

    std::string CodeAsString() {
        char buf[256]={'\0'};                
        snprintf(buf ,256 ,"%d", code);             
        std::string codestr;                 
        codestr.append(buf);                 
        return codestr;                      
    }                                        

    void clear() {
        code = 0;
        body.clear();                        
        header.clear();
    }

    void ConsumeHeader(const std::string& in) {
        header.ParseString(in);
    }

    int code;
    std::string body;                        
    HttpHeader header;  // Response Header
};                                           

}//namespace
#endif

