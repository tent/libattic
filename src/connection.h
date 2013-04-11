#ifndef CONNECTION_H_
#define CONNECTION_H_ 
#pragma once

#include <string>
#define BOOST_NETWORK_ENABLE_HTTPS 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 
#include <boost/foreach.hpp>

#include "response.h"

namespace attic { 

class Connection {
    int InitializeSSLSocket(const std::string& host);
    void SSLLoadCerts();
public:
    Connection(boost::asio::io_service* io_service);
    ~Connection();

    int Initialize(const std::string& url);
    int Close();

    unsigned int Write(boost::asio::streambuf& request);
    void InterpretResponse(Response& out);

private:
    boost::asio::io_service* io_service_;
    boost::asio::ip::tcp::socket* socket_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>* ssl_socket_;
    boost::asio::ssl::context* ctx_;
    bool ssl_;
};


// TODO :: finish this for testing
class AsyncConnection {
public:
    AsyncConnection();
    ~AsyncConnection();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
};

}//namespace
#endif

