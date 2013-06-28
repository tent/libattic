#ifndef PLAINFILEUPLOAD_H_
#define PLAINFILEUPLOAD_H_
#pragma once

#include <string>
#include "connection.h"
#include "accesstoken.h"
#include "downloadpost.h"

namespace attic {


class PlainFileUpload {
    bool BeginRequest();
    bool UploadPostBody(DownloadPost& out);
    bool UploadFile(const std::string& filepath,
                    const std::string& filename,
                    const std::string& filesize);
    bool EndRequest();
    bool InterpretResponse(DownloadPost& out);

    bool WriteFileToConnection(const std::string& filepath);
    bool GenerateDownloadPost(const std::string& filepath, DownloadPost& out);
    bool Push(boost::asio::streambuf& request);
public:
    PlainFileUpload(const AccessToken& at);
    ~PlainFileUpload();

    bool Upload(const std::string& url, 
                const std::string& filepath,
                DownloadPost& out);

private:
    AccessToken access_token_;
    std::string url_;
    std::string boundary_;

    std::string filepath_;

    boost::asio::io_service io_service_;
    Connection* con_;
};

}//namespace 

#endif

