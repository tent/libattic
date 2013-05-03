#include "connectionhandler.h"


namespace attic { 

ConnectionHandler::ConnectionHandler() {
    manager_instance_ = ConnectionManager::GetInstance();
}

ConnectionHandler::~ConnectionHandler() {
    manager_instance_->Release();
}

void ConnectionHandler::SetEntityUrlOnManager(const std::string& entity_url) {
    manager_instance_->Initialize(entity_url);
}
int ConnectionHandler::HttpPost(const std::string& url, 
                                const std::string& post_type,
                                const UrlParams* pParams,
                                const std::string& requestbody,
                                const AccessToken* at, 
                                Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) netlib::EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "POST",
                                at->access_token(),
                                at->hawk_key(),
                                at->app_id(),
                                authheader);
    }

    char len[256] = {'\0'};
    snprintf(len, 256, "%lu", requestbody.size());

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "POST " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: "<< cnst::g_accept_header <<"\r\n";
    request_stream << "Content-Type: " << cnst::g_content_type_header << ";";
    
    if(!post_type.empty()) {
        request_stream << " type=\"";
        request_stream << post_type;
        request_stream << "\""; 
    }
    request_stream << "\r\n";
    request_stream << "Content-Length: " << len << "\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream <<"\r\n";
    //request_stream << "Connection: close\r\n\r\n";
    request_stream << requestbody;

    /*
    std::ostringstream ss;
    ss << &request;
    std::cout <<"ss : " <<  ss.str() << std::endl;
    */

    status = HttpRequest(local_url, request, out);
    return status;
}

int ConnectionHandler::HttpPut(const std::string& url, 
                               const std::string& post_type,
                               const UrlParams* pParams,
                               const std::string& requestbody,
                               const AccessToken* at, 
                               Response& out) {
    int status = ret::A_OK;

    std::cout<<" type : " << post_type << std::endl;
    std::string local_url = url;
    if(pParams) netlib::EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "PUT",
                                at->access_token(),
                                at->hawk_key(),
                                at->app_id(),
                                authheader);
    }
    char len[256] = {'\0'};
    snprintf(len, 256, "%lu", requestbody.size());
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "PUT " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: "<< cnst::g_accept_header <<"\r\n";
    request_stream << "Content-Type: " << cnst::g_content_type_header << ";";
    if(!post_type.empty()) {
        request_stream << " type=\"";
        request_stream << post_type;
        request_stream << "\""; 
    }
    request_stream << "\r\n";
    request_stream << "Content-Length : " << len << "\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream <<"\r\n";
    //request_stream << "Connection: close\r\n\r\n";
    request_stream << requestbody;
/*
    std::ostringstream ss;
    ss << &request;
    std::cout <<"ss : " <<  ss.str() << std::endl;
    */

    status = HttpRequest(local_url, request, out);
    return status;
}

int ConnectionHandler::HttpGet(const std::string& url, 
                               const UrlParams* pParams,
                               const AccessToken* at, 
                               Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) netlib::EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "GET",
                                at->access_token(),
                                at->hawk_key(),
                                at->app_id(),
                                authheader);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: "<< cnst::g_accept_header <<"\r\n";
    request_stream << "Content-Type: " << cnst::g_content_type_header << "\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream <<"\r\n";
    //request_stream << "Connection: close\r\n\r\n";

    status = HttpRequest(local_url, request, out);
    return status;
}

int ConnectionHandler::HttpRequest(const std::string& url, 
                                   boost::asio::streambuf& request,
                                   Response& out) {
    int status = ret::A_OK;
    using namespace boost::asio::ssl;
    try {
        boost::asio::io_service io_service; 
        //Connection sock(&io_service);
        //sock.Initialize(url);
        Connection* sock = manager_instance_->RequestConnection(url);
        if(sock) {
            sock->Write(request);
            sock->InterpretResponse(out);
            manager_instance_->ReclaimConnection(sock);
        }
        else {
            std::cout<<" INVALID SOCKET " << std::endl;
        }

    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return status;
}



} //namespace
