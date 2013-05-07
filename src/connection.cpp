#include "connection.h"

#include "errorcodes.h"
#include "netlib.h"

namespace attic { 

Connection::Connection(boost::asio::io_service* io_service) {
    //io_service_ = io_service;
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

    //io_service_ = NULL;
    ssl_ = false;
}

int Connection::Initialize(const std::string& url) {
    using boost::asio::ip::tcp;
    int status = ret::A_OK;
    socket_ = new tcp::socket(io_service_);

    if(!SetTimeout())
        std::cout<<" failed to set timeout " << std::endl;

    std::string protocol, host, path;
    netlib::ExtractHostAndPath(url, protocol, host, path);

    if(protocol == "https")
        ssl_ = true;

    status = netlib::ResolveHost(io_service_, *socket_, host, ssl_);

    if(ssl_)
        status = InitializeSSLSocket(host);
    if(status == ret::A_OK) {

    }
    else {
        status = ret::A_FAIL_INVALID_IOSERVICE;
    }

    return status;
}

int Connection::Close() {
    int status = ret::A_OK;
    try {
        socket_->close();
    }
    catch(std::exception& e) {
        std::cout<<" Connection close error " << e.what() << std::endl;
    }

    return status;
}

int Connection::InitializeSSLSocket(const std::string& host) {
    using boost::asio::ip::tcp;
    int status = ret::A_OK;

    boost::system::error_code error = boost::asio::error::host_not_found; 
    ctx_ = new boost::asio::ssl::context(io_service_,
                                         boost::asio::ssl::context::sslv23_client);

    // Load Cert
    SSLLoadCerts();

    //ctx_->set_verify_mode(boost::asio::ssl::context::verify_none);
    ctx_->set_verify_mode(boost::asio::ssl::context::verify_peer);
    ssl_socket_ = new boost::asio::ssl::stream<tcp::socket&>(*socket_, *ctx_);
    ssl_socket_->set_verify_callback(boost::asio::ssl::rfc2818_verification(host.c_str()));
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

bool Connection::SetTimeout() {
    struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000; 
    if (setsockopt (socket_->native_handle(),
                    SOL_SOCKET, 
                    SO_SNDTIMEO, 
                    (char *)&timeout, sizeof(timeout)) < 0) {
        std::cout << "setsockopt failed\n";
        return false;
    }
    return true;
}

bool Connection::TestConnection() {
    std::cout<<" TESTING CONNECTION " << std::endl;
    boost::system::error_code error;
    boost::asio::streambuf buf;

    /*
    boost::asio::streambuf req;
    std::ostream req_stream(&req);
    req_stream << "1\r\n\r\n";
    */

    unsigned int bytes_written = 0;
    try {
        if(!ssl_) {
     //       bytes_written = boost::asio::write(*socket_, req, error);
            //std::cout<<" BYTES WRITTEN : " << bytes_written << std::endl;
           
            /*
            struct timeval timeout;      
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000; 
            if (setsockopt (socket_->native_handle(),
                            SOL_SOCKET, 
                            SO_SNDTIMEO, 
                            (char *)&timeout, sizeof(timeout)) < 0)
                std::cout << "setsockopt failed\n";
            else {
                std::cout<<" setsockopt win " << std::endl;
            }

            fd_set readfds;
            int socket_fd = socket_->native_handle();
            FD_ZERO(&readfds);
            FD_SET(socket_->native_handle(), &readfds);
            int sel = select(socket_->native_handle(), &readfds, NULL, NULL, &timeout);
            if(sel < 0 ) 
                std::cout<<" SELECT FAIL " << std::endl;
            std::cout<<"SELECT : " <<  sel << std::endl;

            if(FD_ISSET(socket_->native_handle(), &readfds)) {
                std::cout<<" READ SOMETHING ... " << std::endl;
            }
            else {
                std::cout<<" TIMED OUT " << std::endl;
            }
            char szbuf[256] = {'\0'};
            int result = recv(socket_fd, szbuf, 256, 0);
            std::cout<<" RECV RESULT : " << result << std::endl;

            //boost::asio::read(*socket_, buf, boost::asio::transfer_at_least(1), error);
            //
            */
        }
        else {
            //bytes_written = boost::asio::write(*socket_, req, error);
            //std::cout<<" BYTES WRITTEN : " << bytes_written << std::endl;
            boost::asio::read(*ssl_socket_, buf, boost::asio::transfer_at_least(1), error);

        }
    }
    catch(std::exception& e) {
        std::cout<<" Test Connection exception : " << e.what() << std::endl;
        return false;
    }


    return true;
}

void Connection::SSLLoadCerts() { // Move to connection pool, load once give to all connections
    std::cout<<" LOADING CERTS " << std::endl;
    std::string data;

    std::ifstream ifs;
    ifs.open("config/cacert.pem", std::ios::binary);

    if(ifs.is_open()) {
        ifs.seekg(0, std::ifstream::end);
        unsigned int size = ifs.tellg();
        ifs.seekg(0, std::ifstream::beg);
        
        char* szData = new char[size];
        ifs.read(szData, size);
        data.append(szData, size);

        if(szData) {
            delete[] szData;
            szData = NULL;
        }
        ifs.close();
    }
    else {
        std::cout<<" COULD NOT LOAD SSL CERT" << std::endl;
    }

    // Load Certificate
    BIO *bio;
    X509 *certificate;

    bio = BIO_new(BIO_s_mem());
    BIO_puts(bio, data.c_str());
    certificate = PEM_read_bio_X509(bio, NULL, NULL, NULL);

    // Insert into X509 store
    X509_STORE* ca_store = X509_STORE_new();
    X509_STORE_add_cert(ca_store, certificate);

    X509_free(certificate);
    BIO_free(bio);

    // Load store into context // boost/asio/ssl/context->implementation
    SSL_CTX_set_cert_store(ctx_->impl(), ca_store); // ca_store will be freed when context is destroyed
    //voila
}

} // namespace

