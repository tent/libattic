#ifndef FILEUPLOADER_H_
#define FILEUPLOADER_H_
#pragma once

#include <string>
#include "downloadpost.h"
#include "connection.h"
#include "accesstoken.h"
#include "response.h"


namespace attic { 

// Streams a file into a tent post
class FileUploader {
    bool GenerateDownloadPost(const std::string& filepath, DownloadPost& out);
public:
    FileUploader(const std::string& filepath, 
                 const std::string& posts_feed, 
                 const AccessToken& at);

    ~FileUploader();

    void BeginRequest();
    void ProcessFile();
    void EndRequest(Response& out);
private:
    DownloadPost dlp_;
    Connection* con_;

    std::string posts_feed_;
    std::string boundary_;
};

}//namespace

#endif

