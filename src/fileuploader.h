#ifndef FILEUPLOADER_H_
#define FILEUPLOADER_H_
#pragma once

#include <string>
#include "connection.h"
#include "accesstoken.h"


namespace attic { 

// Streams a file into a tent post
class FileUploader {
public:
    FileUploader(const std::string& filepath, 
                 const std::string& posts_feed, 
                 const AccessToken& at);

    ~FileUploader();
private:
    Connection* con_;

    std::string posts_feed_;
    std::string boundary_;
};

}//namespace

#endif

