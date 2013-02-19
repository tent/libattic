#ifndef MULTIPARTCONSUMER_H_
#define MULTIPARTCONSUMER_H_
#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 

using boost::asio::ip::tcp;

class MultipartConsumer
{
public:
    MultipartConsumer();
    ~MultipartConsumer();

    int ConnectToHost();
    int DisconnectFromHost();

    int PushBodyForm(const std::string& body);
    int PushPartIntoForm(const std::string& buf);
    int SendFooter();

private:
    boost::asio::ssl::stream<tcp::socket&> m_SSL_Socket;
    boost::asio::io_service     m_IO_Service;
    tcp::resolver               m_Resolver;
    tcp::socket                 m_Socket;
};

#endif

