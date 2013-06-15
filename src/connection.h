#ifndef CONNECTION_H_
#define CONNECTION_H_ 
#pragma once

#include <string>
#define BOOST_NETWORK_ENABLE_HTTPS 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 
#include <boost/foreach.hpp>
using boost::asio::ip::tcp;

#include "response.h"

namespace attic { 

class Connection {

    int InitializeSSLSocket(const std::string& host);
    void SSLLoadCerts();

    bool SetTimeout();

    void InterpretResponse(tcp::socket* socket, 
                           Response& resp,
                           bool connection_close = false);

    void InterpretResponse(boost::asio::ssl::stream<tcp::socket&>* socket, 
                           Response& resp,
                           bool connection_close = false);
public:
    Connection(boost::asio::io_service* io_service);
    ~Connection();

    int Initialize(const std::string& url);
    int Close();

    bool TestConnection();

    unsigned int Write(boost::asio::streambuf& request);
    bool InterpretResponse(Response& out);

private:
    //boost::asio::io_service* io_service_;
    boost::asio::io_service io_service_;
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

