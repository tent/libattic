#ifndef NETLIB_H_
#define NETLIB_H_
#pragma once

#define BOOST_NETWORK_ENABLE_HTTPS 

#include <iostream>
#include <string>
#include <list>
#include <string.h>
#include <stdio.h>
//#include <boost/mime.hpp>
#include <boost/network/uri/encode.hpp>
#include <boost/network/protocol/http/client.hpp>

#include <hex.h>        // cryptopp
#include <hmac.h>       // cryptopp
#include <base64.h>     // cryptopp
 
#include "url.h"
#include "urlparams.h"
#include "utils.h"
#include "errorcodes.h"
#include "accesstoken.h"
#include "connectionmanager.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 
using boost::asio::ip::tcp;


using namespace boost::network;


namespace netlib
{
    struct body_handler {
        explicit body_handler(std::string & body)
                                : body(body) {}

        BOOST_NETWORK_HTTP_BODY_CALLBACK(operator(), range, error) {
            std::cout<< " BODY HANDLER HIT BRAH : " << std::endl;
            body.append(boost::begin(range), boost::end(range));

            std::cout<<" just appended to body? : " << body << std::endl;
        }

        std::string & body;
    };

/*
    BOOST_NETWORK_HTTP_BODY_CALLBACK(print_body, range, error) {
        std::cout<< "BODY CALLBACK HIT : " << std::endl;
        if (!error){
            std::cout << "Received " << boost::distance(range) << "bytes." << std::endl;
            std::string b;
            b.append(boost::begin(range), boost::end(range));
            std::cout<<" what i received : " << b << std::endl;
        }
        else
            std::cout << "Error: " << error << std::endl;
    }
    */

/*
    struct my_traits {
        typedef std::string string_type;
        //  typedef std::pair < std::string, string_type > header_type;
        typedef std::string body_type;
    };

    typedef boost::mime::basic_mime<my_traits>  mime_part;
    */
    // Forward Declarations ******************************************************
    static int HttpGet( const std::string& url, 
                        const UrlParams* pParams,
                        const AccessToken* at, 
                        Response& out);

    static int HttpPost( const std::string& url, 
                         const UrlParams* pParams,
                         const AccessToken* at, 
                         Response& out);

    static int HttpPut( const std::string& url, 
                        const UrlParams* pParams,
                        const AccessToken* at, 
                        Response& out);

    static void GenerateHmacSha256(std::string &out);

    static void BuildRequest( const std::string& url,
                              const std::string& requestMethod,
                              const AccessToken* at,
                              http::client::request& reqOut);

    static void BuildMultipartRequest( const std::string& url,
                                       const std::string& requestMethod,
                                       const AccessToken* at,
                                       http::client::request& reqOut);

    static void BuildAuthHeader( const std::string &url, 
                                 const std::string &requestMethod, 
                                 const std::string &macid, 
                                 const std::string &mackey, 
                                 std::string& out);
 
    static void GenerateNonce(std::string &out);

    static void SignRequest( const std::string &request, 
                             const std::string &key, 
                             std::string &out);
 
    static void GenerateHmacSha256(std::string &out);

    static void EncodeAndAppendUrlParams(const UrlParams* pParams, std::string& url);

    static int InterpretResponse( boost::asio::streambuf& response, 
                                  boost::asio::ssl::stream<tcp::socket&>& ssl_sock, 
                                  Response& resp);

    static int InterpretResponse( boost::asio::streambuf& response, 
                                  tcp::socket& socket, 
                                  Response& resp);
    
    // TODO move to hpp
    // Definitions start ***********************************************************
    static int HttpGet( const std::string& url, 
                        const UrlParams* pParams,
                        const AccessToken* at, 
                        Response& out)
    {
        int sstatus = ret::A_OK;

        std::string uri = url;
        EncodeAndAppendUrlParams(pParams, uri);

        http::client::request request;
        BuildRequest( url,
                      "GET",
                      at,
                      request);

        http::client client;
        http::client::response response = client.get(request);

        int st = status(response);
        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);
    
        return sstatus;
    }

    static int HttpPost( const std::string& url, 
                         const UrlParams* pParams,
                         const std::string& requestbody,
                         const AccessToken* at, 
                         Response& out)
    {
        int sstatus = ret::A_OK;

        std::string uri = url;
        EncodeAndAppendUrlParams(pParams, uri);

        http::client::request request;
        BuildRequest( url,
                      "POST",
                      at,
                      request);

        http::client client;

        // No need to set set the content length, that's take care of automatically
        http::client::response response = client.post(request, requestbody);

        int st = status(response);
        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);

        return sstatus;
    }

    static int HttpPut( const std::string& url, 
                        const UrlParams* pParams,
                        const std::string& requestbody,
                        const AccessToken* at, 
                        Response& out)
    {
        int sstatus = ret::A_OK;

        std::string uri = url;
        EncodeAndAppendUrlParams(pParams, uri);

        http::client::request request;
        BuildRequest( url,
                      "PUT",
                      at,
                      request);

        http::client client;
        // No need to set set the content length, that's take care of automatically
        http::client::response response = client.put(request, requestbody);

        int st = status(response);
        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);

        return sstatus;
    }

    static int HttpMultipartRequest( const std::string& requestType, 
                                     const std::string& url, 
                                     const UrlParams* pParams,
                                     const std::string& requestbody,
                                     const AccessToken* at, 
                                     Response& out)
    {
        int sstatus = ret::A_OK;

        if(requestType.empty())
            return ret::A_FAIL_EMPTY_STRING;

        std::string uri = url;
        EncodeAndAppendUrlParams(pParams, uri);

        http::client::request request;
        BuildMultipartRequest( url,
                               requestType,
                               at,
                               request);


/*
        boost::shared_ptr<mime_part> part ( new mime_part ( "text", "plain" ));
        part->set_body ( "This is a test.....\n", 20 );
        part->append_phrase_to_content_type ( "charset", "usascii" );
            std::cout << "PART : " << part << std::endl;

        mime_part mp( "multipart", "multiple" );
        mp.set_body("This is the body of a multipart\n", 32);
        mp.append_part(part);

        std::ostringstream os;
        os << mp;
        std::cout<< mp << std::endl;
        */

        //request << os.str();

        http::client client;
        http::client::response response;
        if(requestType == "POST")
            //response = client.post(request, requestbody, http::_body_handler=print_body);
            response = client.post(request, requestbody);
        else
            //response = client.put(request, requestbody, http::_body_handler=print_body);
            response = client.put(request, requestbody);

        int st = status(response);

        std::cout << " STATUS : " << status(response) << std::endl;
        std::cout << " BODY : " << body(response) << std::endl;

        out.code = status(response);
        out.body = body(response);

        return sstatus;
    }


    // Utility Functions ***********************************************************
    static void EncodeAndAppendUrlParams(const UrlParams* pParams, std::string& url)
    {
        if(pParams)
        {
            std::string enc;
            pParams->SerializeAndEncodeToString(enc);
            url += enc;
        }
    }

    static void BuildRequest( const std::string& url,
                              const std::string& requestMethod,
                              const AccessToken* at,
                              http::client::request& reqOut)
    {
        reqOut.uri(url);
        std::string authheader;
        if(at)
        {
            // Build Auth Header
            BuildAuthHeader( url,
                             requestMethod,
                             at->GetAccessToken(),
                             at->GetMacKey(),
                             authheader );

        }

        reqOut << header("Connection", "close");
        reqOut << header("Accept:", "application/vnd.tent.v0+json" );
        reqOut << header("Content-Type:", "application/vnd.tent.v0+json");

        if(!authheader.empty())
            reqOut << header("Authorization: " , authheader);
    }

    static void BuildMultipartRequest( const std::string& url,
                                       const std::string& requestMethod,
                                       const AccessToken* at,
                                       http::client::request& reqOut)
    {
        reqOut.uri(url);
        std::string authheader;
        if(at)
        {
            // Build Auth Header
            BuildAuthHeader( url,
                             requestMethod,
                             at->GetAccessToken(),
                             at->GetMacKey(),
                             authheader );

        }

        reqOut << header("Connection", "close");
        reqOut << header("Accept:", "application/vnd.tent.v0+json" );
        reqOut << header("Content-Type:", "multipart/form-data");

        if(!authheader.empty())
            reqOut << header("Authorization: " , authheader);
    }

    static int HttpAsioGet( const std::string& url,
                             const AccessToken* at)
                             //Response& resp)

    {

        using namespace boost::asio::ssl;
        try
        {
            // Parse the url, separate the root from the path
            std::string host, path;
            host = "manuel.tent.is";
            path = "/";

            boost::asio::io_service io_service; 

            // Get a list of endpoints corresponding to the server name. 
            tcp::resolver resolver(io_service); 
            tcp::resolver::query query(host, "https");   // <-- HTTPS 
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
            tcp::resolver::iterator end; 

            // Try each endpoint until we successfully establish a connection. 
            tcp::socket socket(io_service); 
            boost::system::error_code error = boost::asio::error::host_not_found; 
            while (error && endpoint_iterator != end) 
            { 
                socket.close(); 
                socket.connect(*endpoint_iterator++, error); 
            } 
            if (error) 
                throw boost::system::system_error(error); 

            // setup an ssl context 
            boost::asio::ssl::context ctx( io_service, 
                                           boost::asio::ssl::context::sslv23_client); 
            ctx.set_verify_mode(boost::asio::ssl::context::verify_none);            // <-- do not verify anything (for non-cdertified ssl keys) 
            boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);       // <-- setup an ssl socket stream based on the socket we already have connected 

            ssl_sock.handshake(boost::asio::ssl::stream_base::client, error); // <-- This is left out in the documentation (Overview/SSL): do not forget the handshake 
            if (error) 
            throw boost::system::system_error(error); 

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "GET " << path << " HTTP/1.1\r\n";
            request_stream << "Host: " << host << "\r\n";
            request_stream << "Accept: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Type: application/vnd.tent.v0+json\r\n";
            request_stream << "Connection: close\r\n\r\n";

            // Send the request.
            //boost::asio::write(socket, request);

            boost::asio::write(ssl_sock, request); // <-- write to the ssl stream 
            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");

            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);

            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                std::cout << "Invalid response\n";
                return 1;
            }
            if (status_code != 200)
            {
                std::cout << "Response returned with status code " << status_code << "\n";
                return 1;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(ssl_sock, response, "\r\n\r\n");

            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
                std::cout << header << "\n";
            std::cout << "\n";

            // Write whatever content we already have to output.
            if (response.size() > 0)
                std::cout << &response;

            // Read until EOF, writing data to output as we go.
            //boost::system::error_code error;
            while (boost::asio::read(ssl_sock, response,
                boost::asio::transfer_at_least(1), error))
            std::cout << &response;
            //if (error != boost::asio::error::eof)
            if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
                throw boost::system::system_error(error);
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }

        return ret::A_OK;
    }

    static void ExtractHostAndPath(const std::string& url, std::string& host, std::string& path)
    {
        std::cout<<" Extracting host and path ... " << std::endl;
        std::cout<< " from url : " << url << std::endl;
        unsigned int pos = url.find(".com");
        std::cout<< " POS : " << pos << std::endl;
        host = url.substr(0, pos+4);
        path = url.substr(pos+4);

        // remove the https
        //host = host.substr(8);
        // remove the http://
        host = host.substr(8);

    }

    static int HttpAsioPut( const std::string& url,
                             const UrlParams* pParams,
                             const std::string& body,
                             const AccessToken* at,
                             Response& resp)
    {
        int statuss = ret::A_OK;
        using namespace boost::asio::ssl;
        try
        {
            std::cout<<"URL : " << url << std::endl;
            // Parse the url, separate the root from the path
            std::string host, path;
            ExtractHostAndPath(url, host, path);
            std::cout<<" HOST : " << host << std::endl;
            std::cout<<" PATH : " << path << std::endl;

            // Define io service
            boost::asio::io_service io_service; 

            // Get a list of endpoints corresponding to the server name. 
            tcp::resolver resolver(io_service); 
            tcp::resolver::query query(host, "https");   // <-- HTTPS 
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
            tcp::resolver::iterator end; 

            // Try each endpoint until we successfully establish a connection. 
            tcp::socket socket(io_service); 
            boost::system::error_code error = boost::asio::error::host_not_found; 
            while (error && endpoint_iterator != end) 
            { 
                socket.close(); 
                socket.connect(*endpoint_iterator++, error); 
            } 
            if (error) 
                throw boost::system::system_error(error); 

            // setup an ssl context 
            boost::asio::ssl::context ctx( io_service, boost::asio::ssl::context::sslv23_client); 
            //do not verify anything (for non-cdertified ssl keys) 
            ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
            //setup an ssl socket stream based on the socket we already have connected
            boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);
            //This is left out in the documentation (Overview/SSL): do not forget the handshake
            ssl_sock.handshake(boost::asio::ssl::stream_base::client, error);

            if (error) 
                throw boost::system::system_error(error); 

            std::string authheader;

            if(at) {
                BuildAuthHeader( url,
                                 "PUT",
                                 at->GetAccessToken(),
                                 at->GetMacKey(),
                                 authheader );
            }

            char buf[256] = {'\0'};
            sprintf(buf, "%lu", body.size());

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "PUT " << path << " HTTP/1.1\r\n";
            request_stream << "Host: " << host << "\r\n";
            request_stream << "Accept: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Type: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Length: " << buf << "\r\n";
            request_stream << "Authorization: " << authheader <<"\r\n";
            request_stream << "Connection: close\r\n\r\n";

            // Now stream the body
            request_stream << body;
            // Send the request.
            //boost::asio::write(socket, request);

            boost::asio::write(ssl_sock, request); // <-- write to the ssl stream 
            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");

            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);

            std::cout<<" STATUS CODE : " << status_code << std::endl;
            resp.code = status_code;

            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                std::cout << "Invalid response\n";
                return 1;
            }
            if (status_code != 200)
            {
                std::cout << "Response returned with status code " << status_code << "\n";
                return 1;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(ssl_sock, response, "\r\n\r\n");

            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
                std::cout << header << "\n";
            std::cout << "\n";

            // Write whatever content we already have to output.
            if (response.size() > 0)
            {
                std::ostringstream strbuf;
                strbuf << &response;
                resp.body = strbuf.str();
                //std::cout << &response;
            }

            // Read until EOF, writing data to output as we go.
            // boost::system::error_code error;
            while (boost::asio::read(ssl_sock, response,
                boost::asio::transfer_at_least(1), error))
            std::cout << &response;
            //if (error != boost::asio::error::eof)
            if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
                throw boost::system::system_error(error);
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }

        return statuss;
    }

    static int HttpAsioPost( const std::string& url,
                             const UrlParams* pParams,
                             const std::string& body,
                             const AccessToken* at,
                             Response& resp)

    {

        using namespace boost::asio::ssl;
        try
        {
            std::cout<<"URL : " << url << std::endl;
            // Parse the url, separate the root from the path
            std::string host, path;
            ExtractHostAndPath(url, host, path);
            std::cout<<" HOST : " << host << std::endl;
            std::cout<<" PATH : " << path << std::endl;

            boost::asio::io_service io_service; 

            // Get a list of endpoints corresponding to the server name. 
            tcp::resolver resolver(io_service); 
            tcp::resolver::query query(host, "https");   // <-- HTTPS 
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
            tcp::resolver::iterator end; 

            // Try each endpoint until we successfully establish a connection. 
            tcp::socket socket(io_service); 
            boost::system::error_code error = boost::asio::error::host_not_found; 
            while (error && endpoint_iterator != end) { 
                socket.close(); 
                socket.connect(*endpoint_iterator++, error); 
            } 
            if (error) 
                throw boost::system::system_error(error); 

            // setup an ssl context 
            boost::asio::ssl::context ctx( io_service, 
                                           boost::asio::ssl::context::sslv23_client); 
            ctx.set_verify_mode(boost::asio::ssl::context::verify_none);            // <-- do not verify anything (for non-cdertified ssl keys) 
            boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);       // <-- setup an ssl socket stream based on the socket we already have connected 

            ssl_sock.handshake(boost::asio::ssl::stream_base::client, error); // <-- This is left out in the documentation (Overview/SSL): do not forget the handshake 
            if (error) 
            throw boost::system::system_error(error); 

            std::string authheader;

            if(at) {
                BuildAuthHeader( url,
                                 "POST",
                                 at->GetAccessToken(),
                                 at->GetMacKey(),
                                 authheader);
            }

            std::cout<<" AUTH HEADER : " << authheader << std::endl;

            char buf[256] = {'\0'};
            sprintf(buf, "%lu", body.size());

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "POST " << path << " HTTP/1.1\r\n";
            request_stream << "Host: " << host << "\r\n";
            request_stream << "Accept: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Type: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Length: " << buf << "\r\n";
            request_stream << "Authorization: " << authheader <<"\r\n";
            request_stream << "Connection: close\r\n\r\n";

            // write the body to the stream
            request_stream << body;
            // Send the request.
            //boost::asio::write(socket, request);

            // write to the ssl stream 
            boost::asio::write(ssl_sock, request); 
            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");

            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);

            std::cout<<" STATUS CODE : " << status_code << std::endl;
            resp.code = status_code;

            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                std::cout << "Invalid response\n";
                return 1;
            }
            if (status_code != 200)
            {
                std::cout << "Response returned with status code " << status_code << "\n";
                return 1;
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(ssl_sock, response, "\r\n\r\n");

            // Response header &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
            // Process the response headers.
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
                std::cout << header << "\n";
            std::cout << "\n";

            // Write whatever content we already have to output.
            if (response.size() > 0)
                std::cout << &response;

            // Response Body &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
            // Read until EOF, writing data to output as we go.
            //boost::system::error_code error;

            std::ostringstream strbuf;
            while (boost::asio::read(ssl_sock, response,
                boost::asio::transfer_at_least(1), error))
                //std::cout << &response;
                strbuf << &response;

            resp.body = strbuf.str();

            //if (error != boost::asio::error::eof)
            if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
                throw boost::system::system_error(error);
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }

        return ret::A_OK;
    }

    static int HttpAsioMultipartPost( const std::string& url,
                                      const UrlParams* pParams,
                                      const std::string& body,
                                      const AccessToken* at,
                                      Response& resp)
    {
        using namespace boost::asio::ssl;
        try
        {
            std::cout<<"URL : " << url << std::endl;
            // Parse the url, separate the root from the path
            std::string host, path;
            ExtractHostAndPath(url, host, path);
            std::cout<<" HOST : " << host << std::endl;
            std::cout<<" PATH : " << path << std::endl;

            boost::asio::io_service io_service; 

            // Get a list of endpoints corresponding to the server name. 
            tcp::resolver resolver(io_service); 
            tcp::resolver::query query(host, "https");   // <-- HTTPS 
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
            tcp::resolver::iterator end; 

            // Try each endpoint until we successfully establish a connection. 
            tcp::socket socket(io_service); 
            boost::system::error_code error = boost::asio::error::host_not_found; 
            while (error && endpoint_iterator != end) { 
                socket.close(); 
                socket.connect(*endpoint_iterator++, error); 
            } 
            if (error) 
                throw boost::system::system_error(error); 

            
            // setup an ssl context 
            boost::asio::ssl::context ctx( io_service, 
                                           boost::asio::ssl::context::sslv23_client); 
            ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
            boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);

            ssl_sock.handshake(boost::asio::ssl::stream_base::client, error);
            if (error) 
                throw boost::system::system_error(error); 

            std::string authheader;

            if(at) {
                BuildAuthHeader( url,
                                 "POST",
                                 at->GetAccessToken(),
                                 at->GetMacKey(),
                                 authheader);
            }

            std::cout<<" AUTH HEADER : " << authheader << std::endl;

            std::string boundary;
            boundary = "Bask33420asdfv";

            // FORM DATA ---------------------------------------------
            char buf2[256] = {"\0"};
            sprintf(buf2, "%lu", body.size());

            // build multipart header
            boost::asio::streambuf requestPart;
            std::ostream part_stream(&requestPart);
            part_stream <<"\r\n--" << boundary << "\r\n";
            part_stream << "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"\r\n";
            part_stream << "Content-Length: " << buf2 << "\r\n";
            //part_stream << "Content-Type: application/octet-stream\r\n";
            part_stream << "Content-Type: application/vnd.tent.v0+json\r\n";
            //part_stream << "Content-Type: text/plain\r\n";
            part_stream << "Content-Transfer-Encoding: binary\r\n\r\n";

            part_stream << body;
            //part_stream << test_data;
            part_stream <<"\r\n--"<< boundary << "--\r\n\r\n";
            
            char buf[256] = {'\0'};
            //sprintf(buf, "%lu", body.size());
            sprintf(buf, "%lu", requestPart.size());

            std::cout<<" SIZE IS : " << buf << std::endl;
            

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "POST " << path << " HTTP/1.1\r\n";
            request_stream << "Host: " << host << "\r\n";
            request_stream << "Accept: application/vnd.tent.v0+json\r\n";
            //request_stream << "Content-Type: application/vnd.tent.v0+json\r\n";
            request_stream << "Content-Type: multipart/form-data; boundary="<< boundary << "\r\n";
            //request_stream << "Expect: 100-continue\r\n";
            //request_stream << "Content-Transfer-Encoding: binary\r\n";
            request_stream << "Content-Length: " << buf << "\r\n";
            request_stream << "Authorization: " << authheader <<"\r\n\r\n";
            //request_stream << "\r\n";

            // write the body to the stream
            //request_stream << body;

            // Send the request.
            //boost::asio::write(socket, request);
            

            /*
            std::ostringstream strbuf;
            strbuf << &requestPart;
            std::ostringstream reqbuf;
            reqbuf << &request;
            std::cout<<"\n\nRequest buffer : " <<std::endl;
            std::cout<<reqbuf.str() << std::endl;
            std::cout<<"\n\nPart buffer : " << std::endl;
            std::cout<<strbuf.str() <<std::endl;
            */

            // Write the request to the socket
            boost::asio::write(ssl_sock, request); 
            //boost::asio::write(socket, request); 

            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.
            boost::asio::streambuf response;
            //boost::asio::read_until(ssl_sock, response, "\r\n");
            //InterpretResponse(response, ssl_sock, resp);

            //if(resp.code == 100)
            {
                std::cout<<" !CALLBACK RESPONSE : " << resp.code << std::endl;
                std::cout<<" BODY : " << resp.body << std::endl;

                // Write the part 
                boost::asio::write(ssl_sock, requestPart); 
                //boost::asio::write(socket, requestPart); 

                boost::asio::read_until(ssl_sock, response, "\r\n");
                //boost::asio::read_until(socket, response, "\r\n");

                //std::cout << " response : " << &response << std::endl;

                InterpretResponse(response, ssl_sock, resp);
                //InterpretResponse(response, socket, resp);

                std::cout<<" CALLBACK RESPONSE : " << resp.code << std::endl;
                std::cout<<" BODY : " << resp.body << std::endl;

            }
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }
    
        return ret::A_OK;
    }


    static int InterpretResponse( boost::asio::streambuf& response, 
                                  boost::asio::ssl::stream<tcp::socket&>& ssl_sock, 
                                  Response& resp)
    {
        // Check that response is OK.
        boost::system::error_code error = boost::asio::error::host_not_found; 

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        std::cout<<" STATUS CODE : " << status_code << std::endl;

        resp.code = status_code;

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            return 1;
        }
        if (status_code != 200)
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            return 1;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(ssl_sock, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";
        std::cout << "\n";

        std::string output_buffer;
        std::cout<<" **************** " << std::endl;
        // Write whatever content we already have to output.
        if (response.size() > 0)
        {
            std::ostringstream strbuf;
            strbuf << &response;
            output_buffer = strbuf.str();
            //resp.body = strbuf.str();
            //std::cout << &response;
        }

        std::cout<<" reading til end of file " << std::endl;
        // Read until EOF, writing data to output as we go.
        //boost::system::error_code error;
        while (boost::asio::read( ssl_sock, 
                                  response,
                                  boost::asio::transfer_at_least(1), 
                                  error))
        {
            std::ostringstream strbuf;
            strbuf << &response;
            output_buffer += strbuf.str();
        }

        std::cout<<" OUTPUT BUFFER : " << output_buffer << std::endl;

        //if (error != boost::asio::error::eof)
        if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
            throw boost::system::system_error(error);

        return ret::A_OK;
    }

    static int InterpretResponse( boost::asio::streambuf& response, 
                                  tcp::socket& socket, 
                                  Response& resp)
    {
        // Check that response is OK.
        boost::system::error_code error = boost::asio::error::host_not_found; 

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        std::cout<<" STATUS CODE : " << status_code << std::endl;

        resp.code = status_code;

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            return 1;
        }
        if (status_code != 200)
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            return 1;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";
        std::cout << "\n";

        std::cout<<" **************** " << std::endl;
        // Write whatever content we already have to output.
        if (response.size() > 0)
        {
            std::ostringstream strbuf;
            strbuf << &response;
            resp.body = strbuf.str();
            std::cout << &response;
        }
        // Read until EOF, writing data to output as we go.
        //boost::system::error_code error;
        while (boost::asio::read(socket, response,
            boost::asio::transfer_at_least(1), error))
        std::cout << &response;
        //if (error != boost::asio::error::eof)
        if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
            throw boost::system::system_error(error);

        return ret::A_OK;
    }

    static void BuildRequestHeader( const std::string& requestMethod,
                                    const std::string& url,
                                    const std::string& boundary,
                                    const AccessToken* pAt,
                                    std::ostream& requeststream)
    {
        std::string host, path;
        ExtractHostAndPath(url, host, path);

        std::string authheader;
        if(pAt)
        {
            netlib::BuildAuthHeader( url,
                                     requestMethod,
                                     pAt->GetAccessToken(),
                                     pAt->GetMacKey(),
                                     authheader);
        }

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.

        //requeststream << "POST " << m_Path << " HTTP/1.1\r\n";
        requeststream << requestMethod << " " << path << " HTTP/1.1\r\n";
        requeststream << "Host: " << host << "\r\n";
        requeststream << "Accept: application/vnd.tent.v0+json\r\n";
        requeststream << "Content-Type: multipart/form-data; boundary="<< boundary << "\r\n";
        requeststream << "Transfer-Encoding: chunked\r\n";
        requeststream << "Connection: close\r\n";
        requeststream << "Authorization: " << authheader <<"\r\n\r\n";
    }

    static void BuildBodyForm( const std::string& body, 
                               const std::string& boundary, 
                               std::ostream& bodystream)
    {
        bodystream <<"\r\n--" << boundary << "\r\n";
        bodystream << "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"\r\n";
        bodystream << "Content-Type: application/vnd.tent.v0+json\r\n";
        bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";
        bodystream << body;
        //bodystream << "\r\n";//\r\n";
        //bodystream <<"\r\n--"<< boundary << "--\r\n\r\n";
        //
        std::cout<<" OUTGOING BODY : " << body << std::endl;
    }

    static void BuildAttachmentForm( const std::string& name, 
                                     const std::string& body,
                                     const std::string& boundary,
                                     std::ostream& bodystream)
    {
        char szSize[256] = {'\0'};
        snprintf(szSize, 256, "%lu", body.size());

        bodystream << "\r\n--" << boundary << "\r\n";
        bodystream << "Content-Disposition: form-data; name=\"attach\"; filename=\"" << name << "\r\n";
        bodystream << "Content-Type: application/octet-stream\r\n";
        bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";

        bodystream << body;

    //    bodystream <<"\r\n--"<< boundary << "--\r\n\r\n";
    }

    static void AddEndBoundry(std::ostream& bodystream, std::string& boundary) 
    {
        bodystream <<"\r\n--"<< boundary << "--\r\n\r\n";
    }

    static void ChunkPart(boost::asio::streambuf& part, std::ostream& outstream)
    {
        std::ostringstream reqbuf;
        reqbuf << &part;
        outstream << std::hex << reqbuf.str().size();
        outstream << "\r\n" << reqbuf.str() << "\r\n";//\r\n0\r\n\r\n";
    }

    static void ChunkEnd(boost::asio::streambuf& part, std::ostream& outstream)
    {
        std::ostringstream reqbuf;
        reqbuf << &part;
        outstream << std::hex << reqbuf.str().size();
        outstream << "\r\n" << reqbuf.str() << "\r\n0\r\n\r\n";
    }

    static int HttpAsioMultipartPost( const std::string& url,
                                      const UrlParams* pParams,
                                      const std::string& body,
                                      const AccessToken* at,
                                      std::list<std::string>& paths,
                                      Response& resp)
    {
        using namespace boost::asio::ssl;
        try
        {
            std::cout<<"URL : " << url << std::endl;
            // Parse the url, separate the root from the path
            std::string host, path;
            ExtractHostAndPath(url, host, path);
            
            boost::asio::io_service io_service; 

            // Get a list of endpoints corresponding to the server name. 
            tcp::resolver resolver(io_service); 
            tcp::resolver::query query(host, "https");   // <-- HTTPS 
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
            tcp::resolver::iterator end; 

            // Try each endpoint until we successfully establish a connection. 
            tcp::socket socket(io_service); 

            /*
            boost::system::error_code errorcode;
            boost::asio::socket_base::keep_alive option(true);

            do
            {
                socket.set_option(option, errorcode);
                std::cout<<" error code : " << errorcode.message() << std::endl;
            }while(errorcode);
            */
            
            boost::system::error_code error = boost::asio::error::host_not_found; 
            while (error && endpoint_iterator != end) { 
                socket.close(); 
                socket.connect(*endpoint_iterator++, error); 
            } 
            if (error) 
                throw boost::system::system_error(error); 

            
            // setup an ssl context 
            boost::asio::ssl::context ctx( io_service, 
                                           boost::asio::ssl::context::sslv23_client); 
            ctx.set_verify_mode(boost::asio::ssl::context::verify_none);
            boost::asio::ssl::stream<tcp::socket&> ssl_sock(socket, ctx);

            ssl_sock.handshake(boost::asio::ssl::stream_base::client, error);
            if (error) 
                throw boost::system::system_error(error); 

            std::string boundary;
            boundary = "Bask33420asdfv";

            // Build request
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            BuildRequestHeader("POST", url, boundary, at, request_stream); 

            // Build Body Form header
            boost::asio::streambuf requestPart;
            std::ostream part_stream(&requestPart);
            BuildBodyForm(body, boundary, part_stream);

            // Chunk the part
            boost::asio::streambuf one;
            std::ostream partbuf(&one);
            ChunkPart(requestPart, partbuf);

            std::string test = "this is a test buffer";
            boost::asio::streambuf attachmentPart;
            std::ostream attch_stream(&attachmentPart);
            BuildAttachmentForm( "testattachment", 
                                 test,
                                 boundary,
                                 attch_stream);

            // Chunk the part
            boost::asio::streambuf two;
            std::ostream partbuftwo(&two);
            ChunkPart(attachmentPart, partbuftwo);

            boost::asio::streambuf attachmentPartTwo;
            std::ostream attch_stream_two(&attachmentPartTwo);
            BuildAttachmentForm( "anothertestattachment",
                                 test,
                                 boundary,
                                 attch_stream_two);

            AddEndBoundry(attch_stream_two, boundary);
            
            // Chunk the end
            boost::asio::streambuf requestEnd;
            std::ostream endstream(&requestEnd);
            ChunkEnd(attachmentPartTwo, endstream);
            
            // Write the request to the socket
            boost::asio::write(ssl_sock, request); 
            boost::asio::write(ssl_sock, one);
            boost::asio::write(ssl_sock, two);
            boost::asio::write(ssl_sock, requestEnd);

            // Get Response
            std::cout<< "getting response " << std::endl;
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");
            std::cout<< "enterpreting response ... " << std::endl;
            InterpretResponse(response, ssl_sock, resp);

            return 0;

/*


            // Read the response status line. The response streambuf will automatically
            // grow to accommodate the entire line. The growth may be limited by passing
            // a maximum size to the streambuf constructor.

            unsigned int count = 0;
            std::list<std::string>::iterator itr = paths.begin(); 
            unsigned int path_count = paths.size();
            std::cout<<" PATH COUNT : " << path_count << std::endl;
            for(;itr!= paths.end(); itr++)
            {
                std::cout<< " create chunk part " << std::endl;
                unsigned int size = utils::CheckFilesize(*itr);
                                                                                      
                // Open and load file into memory                                         
                std::ifstream ifs;                                                        
                ifs.open((*itr).c_str(), std::ifstream::in | std::ifstream::binary);      
                
                char* pBuf = new char[size];                                              
                ifs.read(pBuf, size);                                                     
                ifs.close();                                                              
                
                std::string strBuf;                                                       
                strBuf.append(pBuf, size);                                                
                
                delete[] pBuf;                                                            
                pBuf = NULL;                                                              

                std::string filename;
                utils::ExtractFileName((*itr), filename);
                
                char num[256]={'\0'};
                snprintf(num, 256, "%d", count);
                filename += "_";
                filename += num;

                // Build attachment
                boost::asio::streambuf attachment;
                std::ostream attachmentstream(&attachment);
                BuildAttachmentForm(filename, strBuf, boundary, attachmentstream);

                if(!count)// == path_count-1)
                {
                    std::cout<<" Adding end stream " << std::endl;

                    AddEndBoundry(attachmentstream, boundary);

                    // Chunk the end
                    boost::asio::streambuf partEnd;
                    std::ostream partendstream(&partEnd);
                    ChunkEnd(attachment, partendstream);

                    std::cout<<" write end to socket " << std::endl;


                    boost::system::error_code errorcode;

                    do
                    {
                        boost::asio::write(ssl_sock, partEnd, errorcode); 
                        if(errorcode)
                            std::cout<<errorcode.message()<<std::endl;
                    }
                    while(errorcode);
                    break;
                }
                else
                {

                    // Chunk the part
                    boost::asio::streambuf part;
                    std::ostream parttbuf(&part);
                    ChunkPart(attachment, partbuf);

                    std::cout<<" write to socket " << std::endl;
                    boost::asio::write(ssl_sock, part); 
                }


                count++;
            }                                                                             

            std::cout<<" getting response " << std::endl;
            boost::asio::streambuf response;
            boost::asio::read_until(ssl_sock, response, "\r\n");
            InterpretResponse(response, ssl_sock, resp);
            
            */
        }
        catch (std::exception& e)
        {
            std::cout << "Exception: " << e.what() << "\n";
        }
    

        return ret::A_OK;
    }
    static void BuildAuthHeader( const std::string &url, 
                                 const std::string &requestMethod, 
                                 const std::string &macid, 
                                 const std::string &mackey, 
                                 std::string& out)
    {
        std::string n;
        GenerateNonce(n);

        out.clear();
        //out.append("Authorization: ");
        
        out.append("MAC id=\"");
        out.append(macid.c_str());
        out.append("\", ");

        time_t t = time(0);
        char tb[256];
        snprintf(tb, (sizeof(time_t)*256), "%ld", t);

        out.append("ts=\"");
        out.append(tb);
        out.append("\", ");

        out.append("nonce=\"");
        out.append(n);
        out.append("\", ");

        Url u(url);

        std::string port;
        if(u.HasPort())
            port = u.GetPort();
        else
        {
            if(u.GetScheme().compare(std::string("https")))
                port.append("443");
            else
                port.append("443");
        }

        std::string requestString;
        requestString.append(tb); // time 
        requestString.append("\n");
        requestString.append(n); // nonce 
        requestString.append("\n");
        requestString.append(requestMethod); // method
        requestString.append("\n");
        std::string uri;
        u.GetRequestURI(uri);
        requestString.append(uri); // request uri
        requestString.append("\n");
        requestString.append(u.GetHost()); // host
        requestString.append("\n");
        requestString.append(port); // port
        requestString.append("\n\n");

        std::string signedreq;
        SignRequest(requestString, mackey, signedreq);

        out.append("mac=\"");
        out.append(signedreq.c_str());
        out.append("\"");
        

       // //std::cout << "REQUEST_STRING : " << requestString << std::endl;
       // //std::cout << "AUTH_HEADER : " << out << std::endl;
    }

    static void GenerateNonce(std::string &out)
    {
        out.clear();
        std::string seed;
        for(int i=0; i<3; i++)
            seed+=utils::GenerateChar();

        utils::StringToHex(seed, out);
    }

   static void SignRequest( const std::string &request, 
                             const std::string &key, 
                             std::string &out)
    {
        std::string mac, encoded, som;

        try
        {
      //      //std::cout<< "Key : " << key << std::endl;
            unsigned char szReqBuffer[request.size()];
            memcpy(szReqBuffer, key.c_str(), strlen(key.c_str())+1);

     //       //std::cout<< " BUFFER : " << szReqBuffer << std::endl;
            //CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, request.size());

            CryptoPP::HMAC< CryptoPP::SHA256 > hmac(szReqBuffer, strlen(key.c_str())+1);

            CryptoPP::StringSource( request,
                                    true, 
                                    new CryptoPP::HashFilter(hmac,
                                    new CryptoPP::StringSink(mac)
                                   ) // HashFilter      
                        ); // StringSource

            CryptoPP::StringSource( mac,
                                    true,
                                    new CryptoPP::Base64Encoder(
                                    new CryptoPP::StringSink(som),
                                    false));
        }
        catch(const CryptoPP::Exception& e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        // Hex encoding, ignore this for now
        encoded.clear();

        CryptoPP::StringSource( som, 
                      true,
                      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                    ); // StringSource


    //    //std::cout << "hmac: " << encoded << std::endl; 
    //    //std::cout << "mac : " << mac << std::endl;

        // trim
        size_t found = som.find(std::string("="));
        if (found != std::string::npos)
        {
            som = som.substr(0, found+1);
        }

        out.clear();
        //out = encoded;
        out = som;
    }

    static void GenerateHmacSha256(std::string &out)
    {
        CryptoPP::AutoSeededRandomPool prng;

        CryptoPP::SecByteBlock key(16);
        prng.GenerateBlock(key, key.size());

        std::string plain = "HMAC Test";
        std::string mac, encoded;

        /*********************************\
        \*********************************/

        encoded.clear();

        CryptoPP::StringSource( key, 
                      key.size(), 
                      true,
                      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                    ); // StringSource

        //std::cout << "key: " << encoded << std::endl;
        //std::cout << "plain text: " << plain << std::endl;

        /*********************************\
        \*********************************/

        try
        {
            CryptoPP::HMAC< CryptoPP::SHA256 > hmac(key, key.size());

            CryptoPP::StringSource( plain, 
                          true, 
                          new CryptoPP::HashFilter( hmac,
                                          new CryptoPP::StringSink(mac)
                                        ) // HashFilter      
                        ); // StringSource
        }
        catch(const CryptoPP::Exception& e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        /*********************************\
        \*********************************/

        // Pretty print
        encoded.clear();

        CryptoPP::StringSource( mac, 
                      true,
                      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                    ); // StringSource

        //std::cout << "hmac: " << encoded << std::endl; 

        out.clear();
        out = encoded;
    }

};

#endif

