#ifndef CHUNKREQUEST_H_
#define CHUNKREQUEST_H_
#pragma once

#include <string>

#include "chunkpost.h"
#include "accesstoken.h"
#include "connection.h"
#include "response.h"

namespace attic { 

class ChunkRequest {
    int WriteToSocket(boost::asio::streambuf& buffer);
public:
    ChunkRequest(const std::string& entity,
                 const std::string& posts_feed, 
                 const std::string& post_path, 
                 const std::string& meta_post_id,
                 const AccessToken& at, 
                 const unsigned int group_number);

    ~ChunkRequest();

    void set_parent_post(const ChunkPost& cp);

    void BeginRequest();
    int PushBackChunk(const std::string& chunk_name, 
                      const std::string& chunk,
                      const unsigned int count);
    void EndRequest(Response& out);
private:
    // TODO :: abstract this away, get a socket from connection manager somehow,
    //         pass in a socket from the contructor, or something, use this for now.
    boost::asio::io_service io_service_;
    Connection* socket_;
    std::string boundary_; // multipart boundary

    ChunkPost post_;
    ChunkPost parent_post_;
    AccessToken access_token_;

    std::string request_type_;
    std::string posts_feed_;
    std::string post_path_;
    std::string meta_post_id_;
    std::string url_;
    std::string entity_;

    unsigned int group_number_;
    bool has_parent_;
};

} // namespace
#endif

