#include "multipartconsumer.h"

#include <iostream>

#include "log.h"
#include "errorcodes.h"
#include "netlib.h"

MultipartConsumer::MultipartConsumer(boost::asio::ssl::context& ctx)
    : m_Resolver(m_IO_Service),
      m_Socket(m_IO_Service),
      m_pSSL_Socket(m_Socket, ctx)
{
}

MultipartConsumer::~MultipartConsumer()
{
}

int MultipartConsumer::ConnectToHost(const std::string& url)
{
    int status = ret::A_OK;
    netlib::ExtractHostAndPath(url, m_Host, m_Path);
    
    // Get list of endpoints corresponding to server name
    tcp::resolver::query query(m_Host, "https");
    tcp::resolver::iterator endpoint_itr = m_Resolver.resolve(query);
    tcp::resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    // try each endpoint until successful connection
    while(error && endpoint_itr != end) {
        m_Socket.close();
        m_Socket.connect(*endpoint_itr++, error);
    }

    if(error){
        alog::Log(Logger::ERROR, boost::system::system_error(error).what());
        status = ret::A_FAIL_TCP_ENDPOINT_NOT_FOUND;
    }

    if(status == ret::A_OK){
        // setup an ssl context


    }

    return status;
}

int MultipartConsumer::SSLHandShake()
{
    int status = ret::A_OK;
    // setup ssl context
    boost::asio::ssl::context ctx (m_IO_Service, boost::asio::ssl::context::sslv23_client);
    // do not verify
    ctx.set_verify_mode(boost::asio::ssl::context::verify_none);

    // Setup new ssl socket

    return status;
}

int MultipartConsumer::DisconnectFromHost()
{
    int status = ret::A_OK;

    return status;
}

int MultipartConsumer::PushBodyForm(const std::string& body)
{
    int status = ret::A_OK;

    return status;
}

int MultipartConsumer::PushPartIntoForm(const std::string& buf)
{
    int status = ret::A_OK;

    return status;
}

int SendFooter()
{
    int status = ret::A_OK;

    return status;
}


