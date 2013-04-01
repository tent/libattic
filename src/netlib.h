#ifndef NETLIB_H_
#define NETLIB_H_
#pragma once

#define BOOST_NETWORK_ENABLE_HTTPS 

#include <iostream>
#include <string>
#include <list>

#include <string.h>
#include <stdio.h>

#include <boost/timer/timer.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 
#include <boost/foreach.hpp>

using boost::asio::ip::tcp;

#include <hex.h>        // cryptopp
#include <hmac.h>       // cryptopp
#include <sha.h>
#include <base64.h>     // cryptopp
 
#include "url.h"
#include "urlparams.h"
#include "utils.h"
#include "response.h"
#include "errorcodes.h"
#include "accesstoken.h"
#include "httpheader.h"
#include "event.h"
#include "connection.h"
#include "constants.h"


namespace attic { namespace netlib {



static int HttpRequest(const std::string& url, 
                       boost::asio::streambuf& request,
                       Response& out);
static int HttpGet( const std::string& url, 
                    const UrlParams* pParams,
                    const AccessToken* at, 
                    Response& out);

static int HttpHead( const std::string& url, 
                     const UrlParams* pParams,
                     const AccessToken* at, 
                     Response& out);

static int HttpPost( const std::string& url, 
                     const std::string& post_type,
                     const UrlParams* pParams,
                     const std::string& requestbody,
                     const AccessToken* at, 
                     Response& out);

static int HttpPut( const std::string& url, 
                    const UrlParams* pParams,
                    const std::string& requestbody,
                    const AccessToken* at, 
                    Response& out);


static int HttpAsioGetAttachment( const std::string& url, 
                                  const UrlParams* pParams,
                                  const AccessToken* at, 
                                  Response& out);

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

static void ExtractHostAndPath( const std::string& url, 
                                std::string& protocol, 
                                std::string& host, 
                                std::string& path );

static void EncodeAndAppendUrlParams( const UrlParams* pParams, std::string& url );

static int ResolveHost( boost::asio::io_service& io_service, 
                        tcp::socket& socket,
                        const std::string& host,
                        const bool ssl = true);

static std::string UriEncode(const std::string & sSrc);


static void InterpretResponse(tcp::socket* socket, Response& resp);
static void InterpretResponse(boost::asio::ssl::stream<tcp::socket&>* socket, Response& resp);

static int GetStatusCode(boost::asio::streambuf& buf);
static void ProcessResponseHeaders(boost::asio::streambuf& buf, Response& resp);
static void ProcessResponseBody(boost::asio::streambuf& buf, 
                                tcp::socket* socket, 
                                bool chunked,
                                Response& resp);

static void DeChunkString(std::string& in, std::string& out) {
    utils::split splitbody;

    // de-chunk the body
    utils::split outsplit;
    std::string delim = "\r\n";
    utils::SplitStringSubStr(in, delim, outsplit);
    
    for(unsigned int i=0; i<outsplit.size(); i+=2)
    {
        std::string chunksize = outsplit[i];
        int c_size;
        sscanf(chunksize.c_str(), "%x", &c_size);

        if(c_size >= 0 && (i+1)<outsplit.size()) {
            out.append(outsplit[i+1].c_str(), c_size);
        }
    }
}

static int HttpGet(const std::string& url, 
                   const UrlParams* pParams,
                   const AccessToken* at, 
                   Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader( local_url,
                                 "GET",
                                 at->GetAccessToken(),
                                 at->GetMacKey(),
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
    request_stream << "Connection: close\r\n\r\n";

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpGetAttachment (const std::string& url, 
                              const UrlParams* pParams,
                              const AccessToken* at, 
                              Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader( local_url,
                                 "GET",
                                 at->GetAccessToken(),
                                 at->GetMacKey(),
                                 authheader);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: "<< cnst::g_accept_octet_header <<"\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream << "Connection: close\r\n\r\n";

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpHead(const std::string& url, 
                    const UrlParams* pParams,
                    const AccessToken* at, 
                    Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "HEAD",
                                at->GetAccessToken(),
                                at->GetMacKey(),
                                authheader);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "HEAD " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: "<< cnst::g_accept_header <<"\r\n";
    request_stream << "Content-Type: " << cnst::g_content_type_header << "\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream << "Connection: close\r\n\r\n";

    /*
    std::ostringstream ss;
    ss << &request;
    std::cout <<"ss : " <<  ss.str() << std::endl;
    */

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpPost(const std::string& url, 
                    const std::string& post_type,
                    const UrlParams* pParams,
                    const std::string& requestbody,
                    const AccessToken* at, 
                    Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "POST",
                                at->GetAccessToken(),
                                at->GetMacKey(),
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
    request_stream << "Connection: close\r\n\r\n";
    request_stream << requestbody;

    /*
    std::ostringstream ss;
    ss << &request;
    std::cout <<"ss : " <<  ss.str() << std::endl;
    */

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpPut(const std::string& url, 
                   const std::string& post_type,
                   const UrlParams* pParams,
                   const std::string& requestbody,
                   const AccessToken* at, 
                   Response& out) {
    int status = ret::A_OK;

    std::cout<<" type : " << post_type << std::endl;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "PUT",
                                at->GetAccessToken(),
                                at->GetMacKey(),
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
    request_stream << "Connection: close\r\n\r\n";
    request_stream << requestbody;
/*
    std::ostringstream ss;
    ss << &request;
    std::cout <<"ss : " <<  ss.str() << std::endl;
    */

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpDelete(const std::string& url, 
                      const UrlParams* pParams,
                      const AccessToken* at, 
                      Response& out) {
    int status = ret::A_OK;
    std::string local_url = url;
    if(pParams) EncodeAndAppendUrlParams(pParams, local_url);

    std::string protocol, host, path;
    ExtractHostAndPath(local_url, protocol, host, path);

    std::string authheader;
    if(at) {
        netlib::BuildAuthHeader(local_url,
                                "DELETE",
                                at->GetAccessToken(),
                                at->GetMacKey(),
                                authheader);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "DELETE " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: application/vnd.tent.v0+json\r\n";
    if(!authheader.empty())
        request_stream << "Authorization: " << authheader <<"\r\n";
    request_stream << "Connection: close\r\n\r\n";

    status = HttpRequest(local_url, request, out);
    return status;
}

static int HttpRequest(const std::string& url, 
                       boost::asio::streambuf& request,
                       Response& out) {
    int status = ret::A_OK;
    using namespace boost::asio::ssl;
    try {
        boost::asio::io_service io_service; 

        Connection sock(&io_service);
        sock.Initialize(url);
        sock.Write(request);
        sock.InterpretResponse(out);
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return status;
}

// Utility Functions ***********************************************************
static void EncodeAndAppendUrlParams(const UrlParams* pParams, std::string& url)
{
    if(pParams) {
        std::string enc;
        pParams->SerializeAndEncodeToString(enc);
        url += enc;
    }
}

static void ExtractHostAndPath( const std::string& url, 
                                std::string& protocol,
                                std::string& host, 
                                std::string& path)
{
    std::string uri = url;
    int left = 0;
    left = uri.find("http");
    if(left != std::string::npos){
        left = 7;
        if(uri[4] == 's') { 
            protocol = "https";
            left += 1;
        } 
        else
            protocol = "http";
    }
    else
        left = 0;

    int right = uri.find("/", left);
    if(right == std::string::npos) {
        utils::CheckUrlAndAppendTrailingSlash(uri);
    }
    right = uri.find("/", left);

    int diff = right - left;
    host = uri.substr(left, diff);
    path = uri.substr(right);
}

static void InterpretResponse(tcp::socket* socket, Response& resp) {
    boost::asio::streambuf response;
    boost::asio::read_until(*socket, response, "\r\n");
    resp.code = GetStatusCode(response);

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(*socket, response, "\r\n\r\n");

    ProcessResponseHeaders(response, resp);
    bool chunked = false;
    if(resp.header["transfer-encoding"].find("chunked") != std::string::npos)
        chunked = true;

    boost::system::error_code error;
    std::string output_buffer;
    // Write whatever content we already have to output.
    if (response.size() > 0) {
        std::ostringstream strbuf;
        strbuf << &response;
        output_buffer = strbuf.str();
    }

    // Read until EOF, writing data to output as we go.
    //boost::system::error_code error;
    while (boost::asio::read(*socket, 
                             response,
                             boost::asio::transfer_at_least(1), 
                             error)) {
        std::ostringstream strbuf;
        strbuf << &response;
        output_buffer += strbuf.str();
    }

    std::string dechunked;
    if(chunked){
        DeChunkString(output_buffer, dechunked);
        output_buffer = dechunked;
    }

    resp.body = output_buffer;

    if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
        throw boost::system::system_error(error);

}

static void InterpretResponse(boost::asio::ssl::stream<tcp::socket&>* socket, Response& resp) {
    boost::asio::streambuf response;
    boost::asio::read_until(*socket, response, "\r\n");
    resp.code = GetStatusCode(response);

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(*socket, response, "\r\n\r\n");

    ProcessResponseHeaders(response, resp);
    bool chunked = false;
    if(resp.header["transfer-encoding"].find("chunked") != std::string::npos)
        chunked = true;

    boost::system::error_code error;
    std::string output_buffer;
    // Write whatever content we already have to output.
    if (response.size() > 0) {
        std::ostringstream strbuf;
        strbuf << &response;
        output_buffer = strbuf.str();
    }

    // Read until EOF, writing data to output as we go.
    //boost::system::error_code error;
    while (boost::asio::read(*socket, 
                             response,
                             boost::asio::transfer_at_least(1), 
                             error)) {
        std::ostringstream strbuf;
        strbuf << &response;
        output_buffer += strbuf.str();
    }

    std::string dechunked;
    if(chunked){
        DeChunkString(output_buffer, dechunked);
        output_buffer = dechunked;
    }

    resp.body = output_buffer;

    if (error != boost::asio::error::eof && error != boost::asio::error::shut_down)
        throw boost::system::system_error(error);
}

static int GetStatusCode(boost::asio::streambuf& buf) {
    std::istream response_stream(&buf);
    std::string http_version;
    response_stream >> http_version;

    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        std::cout<<" Invalid stream or NON http : http_version : " << http_version << std::endl;

    return status_code;
}

static void ProcessResponseHeaders(boost::asio::streambuf& buf, Response& resp) {
    // Process the response headers.
    std::istream response_stream(&buf);
    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
        resp.ConsumeHeader(header + "\n");
        //std::cout << header << "\n";
    }
}

static void BuildRequestHeader( const std::string& requestMethod,
                                const std::string& url,
                                const std::string& boundary,
                                const AccessToken* pAt,
                                std::ostream& requeststream) {
    std::string protocol, host, path;
    ExtractHostAndPath( url, 
                        protocol, 
                        host, 
                        path);

    std::string authheader;
    if(pAt) {
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

static void BuildBodyForm( const std::string& post_type,
                           const std::string& body, 
                           const std::string& boundary, 
                           std::ostream& bodystream) {
    bodystream <<"\r\n--" << boundary << "\r\n";
    char szSize[256] = {'\0'};
    snprintf(szSize, 256, "%lu", body.size());
    bodystream << "Content-Disposition: form-data; name=\"post\"; filename=\"post.json\"\r\n";
    bodystream << "Content-Length: " << szSize << "\r\n";
    bodystream << "Content-Type: " << cnst::g_content_type_header << ";";
    if(!post_type.empty()) {
        bodystream << " type=\"";
        bodystream << post_type;
        bodystream << "\""; 
    }
    bodystream << "\r\n";

    bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";
    bodystream << body;
}

static void BuildAttachmentForm( const std::string& name, 
                                 const std::string& body,
                                 const std::string& boundary,
                                 unsigned int attachmentnumber,
                                 std::ostream& bodystream) {

    std::cout<<" ATTACHMENT NAME : " << name << std::endl;
    std::cout<<" BODY SIZE : " << body.size() << std::endl;
    char szSize[256] = {'\0'};
    snprintf(szSize, 256, "%lu", body.size());
    char szAttachmentCount[256] = {'\0'};
    snprintf(szAttachmentCount, 256, "%d", attachmentnumber);
    std::cout<<" attachment count : " << szAttachmentCount << std::endl;

    bodystream << "\r\n--" << boundary << "\r\n";
    bodystream << "Content-Disposition: form-data; name=\"attach[" << szAttachmentCount << "]\"; filename=\"" << name << "\"\r\n";
    bodystream << "Content-Length: " << szSize << "\r\n";
    bodystream << "Content-Type: application/octet-stream\r\n";
    bodystream << "Content-Transfer-Encoding: binary\r\n\r\n";

    bodystream << body;
}

static void AddEndBoundry(std::ostream& bodystream, const std::string& boundary) {
    bodystream <<"\r\n--"<< boundary << "--\r\n\r\n";
}

static void ChunkPart(boost::asio::streambuf& part, std::ostream& outstream) {
    std::ostringstream reqbuf;
    reqbuf << &part;

    outstream << std::hex << reqbuf.str().size();
    outstream << "\r\n" << reqbuf.str() << "\r\n";//\r\n0\r\n\r\n";
}

static void ChunkEnd(boost::asio::streambuf& part, std::ostream& outstream) {
    std::ostringstream reqbuf;
    reqbuf << &part;
    outstream << std::hex << reqbuf.str().size();
    outstream << "\r\n" << reqbuf.str() << "\r\n0\r\n\r\n";
}

static int ResolveHost(boost::asio::io_service& io_service, 
                       tcp::socket& socket,
                       const std::string& host,
                       const bool ssl)
{
    int status = ret::A_OK;

    std::cout<<" host " << host << std::endl;
    std::string resolve_host = host;
    // Extract port (if one exists)
    std::string port;
    int pos = host.find(":");
    if(pos != std::string::npos) { 
        port = host.substr(pos+1);
        resolve_host = host.substr(0, pos);
    }

    boost::system::error_code error = boost::asio::error::host_not_found; 

    // Get a list of endpoints corresponding to the server name. 
    tcp::resolver resolver(io_service); 

    std::string protocol("https");
    if(!ssl)
        protocol = "http";

    if(port.empty())
        port = protocol;
    
    std::cout<<" HOST : " << resolve_host << std::endl;
    std::cout<<" PORT : " << port << std::endl;

    tcp::resolver::query query(resolve_host, port);
    //tcp::resolver::query query(host, protocol, boost::asio::ip::resolver_query_base::numeric_service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query); 
    tcp::resolver::iterator end; 
    
    while (error && endpoint_iterator != end) { 
        socket.close(); 
        socket.connect(*endpoint_iterator++, error); 
    } 

    if (error) {
        std::cout<<" Resolve host error : " << boost::system::system_error(error).what() << std::endl;
        //throw boost::system::system_error(error); 
        status = ret::A_FAIL_TCP_ENDPOINT_NOT_FOUND;
    }

    return status;
}

static void BuildAuthHeader(const std::string &url, 
                            const std::string &requestMethod, 
                            const std::string &macid, 
                            const std::string &mackey, 
                            std::string& out) {
    std::string n;
    GenerateNonce(n);

    out.clear();
    
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
        port = u.port();
    else {
        std::cout<<" scheme : " << u.scheme() << std::endl;
        if(u.scheme() == std::string("https"))
            port.append("443");
        else
            port.append("80");
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
    requestString.append(u.host()); // host
    requestString.append("\n");
    requestString.append(port); // port
    requestString.append("\n\n");

    std::string signedreq;
    SignRequest(requestString, mackey, signedreq);

    out.append("mac=\"");
    out.append(signedreq.c_str());
    out.append("\"");
    
    std::cout << "REQUEST_STRING : " << requestString << std::endl;
    std::cout << "AUTH_HEADER : " << out << std::endl;
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

    try {
        unsigned char szReqBuffer[request.size()];
        memcpy(szReqBuffer, key.c_str(), strlen(key.c_str())+1);

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
    catch(const CryptoPP::Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    // Hex encoding, ignore this for now
    encoded.clear();

    CryptoPP::StringSource( som, 
                  true,
                  new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)) // HexEncoder
                ); // StringSource
    // trim
    size_t found = som.find(std::string("="));
    if (found != std::string::npos) {
        som = som.substr(0, found+1);
    }

    out.clear();
    //out = encoded;
    out = som;
}

// TODO replace this uri encode implementation with something
static const char SAFE[256] = {
/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

static std::string UriEncode(const std::string & sSrc) {
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
    const int SRC_LEN = sSrc.length();
    unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
    unsigned char * pEnd = pStart;
    const unsigned char * const SRC_END = pSrc + SRC_LEN;
    
    for (; pSrc < SRC_END; ++pSrc) {
        if (SAFE[*pSrc])
            *pEnd++ = *pSrc;
        else {
            // escape this char
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
    }
    
    std::string sResult((char *)pStart, (char *)pEnd);
    delete [] pStart;
    return sResult;
}


}} //namespace
#endif

