#ifndef PLAINFILEUPLOAD_H_
#define PLAINFILEUPLOAD_H_
#pragma once

#include <string>
#include "netlib.h"
#include "downloadpost.h"

namespace attic {

class Connection;

class PlainFileUpload {
    bool GenerateDownloadPost(const std::string& filepath, DownloadPost& out);
    bool Push(boost::asio::streambuf& request);
public:
    PlainFileUpload(const std::string& url);
    ~PlainFileUpload();

    void Upload(const std::string& filepath);

private:
    std::string url_;
    std::string boundary_;

    Connection* con_;


};

}//namespace 

#endif

