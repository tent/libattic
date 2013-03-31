#ifndef ATTICSOCKET_H_
#define ATTICSOCKET_H_
#pragma once

#include <string>
#define BOOST_NETWORK_ENABLE_HTTPS 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 
#include <boost/foreach.hpp>

#include "response.h"

namespace attic { 

class AtticSocket {
    int InitializeSSLSocket();


public:
    AtticSocket(boost::asio::io_service* io_service);
    ~AtticSocket();

    int Initialize(const std::string& url);

    void WriteRequest(boost::asio::streambuf& request);
    void InterpretResponse(Response& out);

private:
    boost::asio::io_service* io_service_;
    boost::asio::ip::tcp::socket* socket_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>* ssl_socket_;
    boost::asio::ssl::context* ctx_;
    bool ssl_;
};

}//namespace
#endif

