#include "connectionmanager.h"

#include "errorcodes.h"

namespace attic { 

int ConnectionManager::Initialize(const std::string& host_url) {
    int status = ret::A_OK;

    // Start up io service
    boost::system::error_code ec;
    try {
        io_service_.run(ec);
    }
    catch(std::exception& e) {
        std::cout<<" Failed to init connection manager " << e.what() << std::endl;
    }

    // save host
    host_url_ = host_url;
    return status;
}

int ConnectionManager::Shutdown() {
    int status = ret::A_OK;

    return status;
}

Connection* ConnectionManager::RequestConnection() {
    pool_mtx_.Lock();
    // Retrieve Connection object
    //Connection* con = open_pool_.PopFront();
    pool_mtx_.Unlock();

    // Check if connection is still alive ... just do a head request,
    //  - if yes, return
    //  - if not, re-connect then return
}

void ConnectionManager::ReclaimConnection(Connection* socket) {
}

}//namespace
