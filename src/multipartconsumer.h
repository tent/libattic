#ifndef MULTIPARTCONSUMER_H_
#define MULTIPARTCONSUMER_H_
#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 

using boost::asio::ip::tcp;

class AccessToken;

class MultipartConsumer
{
    void BuildRequestHeader( const std::string& requestMethod,
                             const std::string& contentLength, 
                             const AccessToken* pAt,
                             std::ostream& requeststream);

    void BuildBodyForm(const std::string& body, std::ostream& bodystream);

    void BuildAttachmentForm( const std::string& name,
                              const std::string& body, 
                              std::ostream& attachmentstream);

    void BuildFooter(std::ostream& endstream);

    int SSLHandShake();
public:
    MultipartConsumer();
    ~MultipartConsumer();

    int ConnectToHost(const std::string& url);
    int DisconnectFromHost();

    int PushBodyForm( const std::string& requestMethod, 
                      const AccessToken* at, 
                      const std::string& body);

    int PushAttachmentForm(const std::string& name, const std::string& body);

    int PushPartIntoForm(const std::string& buf);
    int SendFooter();

private:
    boost::asio::ssl::stream<tcp::socket&> m_SSL_Socket;
    boost::asio::ssl::context m_Ctx;

    boost::asio::io_service     m_IO_Service;
    tcp::resolver               m_Resolver;
    tcp::socket                 m_Socket;

    std::string                 m_Url;
    std::string                 m_Host;
    std::string                 m_Path;
    std::string                 m_Boundary;
};

#endif

