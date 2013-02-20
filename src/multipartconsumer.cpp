#include "multipartconsumer.h"

#include <iostream>
#include <cstdio>

#include "log.h"
#include "errorcodes.h"
#include "accesstoken.h"
#include "netlib.h"

MultipartConsumer::MultipartConsumer()
    : m_Resolver(m_IO_Service),
      m_Socket(m_IO_Service),
      m_Ctx(m_IO_Service, boost::asio::ssl::context::sslv23_client),
      m_SSL_Socket(m_Socket, m_Ctx)
{

}

MultipartConsumer::~MultipartConsumer()
{
}

int MultipartConsumer::ConnectToHost(const std::string& url)
{
    int status = ret::A_OK;
    m_Url = url;
    netlib::ExtractHostAndPath(m_Url, m_Host, m_Path);
    
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

    if(status == ret::A_OK)
        status = SSLHandShake();

    return status;
}

int MultipartConsumer::SSLHandShake()
{
    int status = ret::A_OK;

    boost::system::error_code error = boost::asio::error::host_not_found;
    m_SSL_Socket.handshake(boost::asio::ssl::stream_base::client, error);
    if(error){
        alog::Log(Logger::ERROR, boost::system::system_error(error).what());
        status = ret::A_FAIL_SSL_HANDSHAKE;
    }

    return status;
}

int MultipartConsumer::DisconnectFromHost()
{
    int status = ret::A_OK;

    return status;
}

void MultipartConsumer::BuildBodyForm(const std::string& body, std::ostream& bodystream)
{
    char szSize[256] = {'\0'};
    snprintf(szSize, 256, "%lu", body.size());

    bodystream << "\r\n--" << m_Boundary << "\r\n";
    bodystream << "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"\r\n";
    bodystream << "Content-Length: " << szSize << "\r\n";
    bodystream << "Content-Type: application/vnd.tent.v0+json\r\n";
    bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";

    bodystream << body;
}

void MultipartConsumer::BuildAttachmentForm( const std::string& name, 
                                             const std::string& body,
                                             std::ostream& bodystream)
{
    char szSize[256] = {'\0'};
    snprintf(szSize, 256, "%lu", body.size());

    bodystream << "\r\n--" << m_Boundary << "\r\n";
    bodystream << "Content-Disposition: form-data; name=\"attach\"; filename=\"" << name << "\r\n";
    bodystream << "Content-Length: " << szSize << "\r\n";
    bodystream << "Content-Type: application/octet-stream\r\n";
    bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";

    bodystream << body;
}

void MultipartConsumer::BuildFooter(std::ostream& endstream)
{
    endstream <<"\r\n--"<< m_Boundary << "--\r\n\r\n";
}

void MultipartConsumer::BuildRequestHeader( const std::string& requestMethod,
                                            const std::string& contentLength, 
                                            const AccessToken* pAt,
                                            std::ostream& requeststream)
{
    std::string authheader;
    if(pAt)
    {
        netlib::BuildAuthHeader( m_Url,
                                 requestMethod,
                                 pAt->GetAccessToken(),
                                 pAt->GetMacKey(),
                                 authheader);
    }

    //requeststream << "POST " << m_Path << " HTTP/1.1\r\n";
    requeststream << requestMethod << " " << m_Path << " HTTP/1.1\r\n";
    requeststream << "Host: " << m_Host << "\r\n";
    requeststream << "Accept: application/vnd.tent.v0+json\r\n";
    requeststream << "Content-Type: multipart/form-data; boundary="<< m_Boundary << "\r\n";
    requeststream << "Content-Length: " << contentLength << "\r\n";
    requeststream << "Authorization: " << authheader <<"\r\n\r\n";
}

int MultipartConsumer::PushBodyForm( const std::string& requestMethod, 
                                    const AccessToken* at, 
                                    const std::string& body)
{
    int status = ret::A_OK;
    // TODO::Generate Random Boundary
    m_Boundary = "Bask33420asdfvkasdf12312";

    // Build body form
    boost::asio::streambuf bodypart;
    std::ostream bodystream(&bodypart);
    BuildBodyForm(body, bodystream);

    char szSize[256] = {'\0'};
    snprintf(szSize, 256, "%lu", bodypart.size());

    // Build Request Header
    boost::asio::streambuf request;
    std::ostream requeststream(&request);
    BuildRequestHeader(requestMethod, szSize, at, requeststream);
    
    // Write request header to socket
    boost::asio::write(m_SSL_Socket, request);

    // Write body part
    boost::asio::write(m_SSL_Socket, bodypart);

    return status;
}

int MultipartConsumer::PushAttachmentForm(const std::string& name, const std::string& body)
{
    int status = ret::A_OK;

    boost::asio::streambuf attachment;
    std::ostream attachmentstream(&attachment);
    BuildAttachmentForm(name, body, attachmentstream);

    boost::asio::write(m_SSL_Socket, attachment);

    return status;
}

int MultipartConsumer::SendFooter()
{
    int status = ret::A_OK;
    boost::asio::streambuf footer;
    std::ostream footerstream(&footer);
    BuildFooter(footerstream);

    boost::asio::write(m_SSL_Socket, footer);

    return status;
}


