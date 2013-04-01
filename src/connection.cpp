#include "connection.h"

#include "errorcodes.h"
#include "netlib.h"

namespace attic { 

Connection::Connection(boost::asio::io_service* io_service) {
    io_service_ = io_service;
    socket_ = NULL;
    ssl_socket_ = NULL;
    ctx_ = NULL;
    ssl_ = false;
}

Connection::~Connection() {
    if(socket_) {
        delete socket_;
        socket_ = NULL;
    }

    if(ssl_socket_) {
        delete ssl_socket_;
        ssl_socket_ = NULL;
    }

    if(ctx_) {
        delete ctx_;
        ctx_ = NULL;
    }

    io_service_ = NULL;
    ssl_ = false;
}

int Connection::Initialize(const std::string& url) {
    using boost::asio::ip::tcp;
    int status = ret::A_OK;
    if(io_service_) {
        socket_ = new tcp::socket(*io_service_);

        std::string protocol, host, path;
        netlib::ExtractHostAndPath(url, protocol, host, path);

        if(protocol == "https")
            ssl_ = true;

        status = netlib::ResolveHost(*io_service_, *socket_, host, ssl_);
        if(status == ret::A_OK) {

        }
    }
    else {
        status = ret::A_FAIL_INVALID_IOSERVICE;
    }

    return status;
}


int Connection::InitializeSSLSocket() {
    using boost::asio::ip::tcp;
    int status = ret::A_OK;

    boost::system::error_code error = boost::asio::error::host_not_found; 
    ctx_ = new boost::asio::ssl::context(*io_service_,
                                         boost::asio::ssl::context::sslv23_client);

    ctx_->set_verify_mode(boost::asio::ssl::context::verify_none);
    ssl_socket_ = new boost::asio::ssl::stream<tcp::socket&>(*socket_, *ctx_);
    ssl_socket_->handshake(boost::asio::ssl::stream_base::client, error);

    if(error) {
        throw boost::system::system_error(error); 
        status = ret::A_FAIL_SSL_HANDSHAKE;
    }

    return status;
}

unsigned int Connection::Write(boost::asio::streambuf& request) {
    unsigned int bytes_written = 0;
    boost::system::error_code error;
    if(ssl_) 
        bytes_written = boost::asio::write(*ssl_socket_, request, error);
    else
        bytes_written = boost::asio::write(*socket_, request, error);

    if(error)
        throw boost::system::system_error(error); 

    return bytes_written;
}

void Connection::InterpretResponse(Response& out) {
    if(ssl_)
        netlib::InterpretResponse(ssl_socket_, out);
    else
        netlib::InterpretResponse(socket_, out);
}

} // namespace

