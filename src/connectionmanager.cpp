#include "connectionmanager.h"

#include "errorcodes.h"
#include "netlib.h"

namespace attic { 

ConnectionManager* ConnectionManager::instance_ = NULL;
bool ConnectionManager::initialized_ = false;
int ConnectionManager::ref_ = 0;

ConnectionManager::ConnectionManager() {}
ConnectionManager::~ConnectionManager() {}

int ConnectionManager::Shutdown() {
    int status = ret::A_OK;
    if(instance_){
        delete instance_;
        instance_ = NULL;
    }
    return status;
}
void ConnectionManager::Release() { 
    ref_--;
    if(ref_ <= 0) { 
        //Shutdown();
    }
}

ConnectionManager* ConnectionManager::instance() {
    if(!instance_)
        instance_ = new ConnectionManager();
    ref_++;
    return instance_;
}

int ConnectionManager::Initialize(const std::string& host_url) {
    int status = ret::A_OK;
    if(!host_url.empty()) {
        // Start up io service
        boost::system::error_code ec;
        try {
            io_service_.run(ec);
        }
        catch(std::exception& e) {
            std::cout<<" Failed to init connection manager " << e.what() << std::endl;
            status = ret::A_FAIL_SUBSYSTEM_NOT_INITIALIZED;
        }

        if(status == ret::A_OK) {
            // save host
            host_url_ = host_url;
            initialized_ = true;
            std::cout<<" Connection manager initialized ... " << std::endl;
        }
    }
    else {
        status = ret::A_FAIL_EMPTY_STRING;
    }

    return status;
}

Connection* ConnectionManager::RequestConnection(const std::string& url) {
    Connection* con = NULL;
    pool_mtx_.Lock();
    if(initialized_) {
        for(;;) {
            if(pool_.empty()) 
                pool_.ExtendPool(&io_service_, host_url_);
            // Retrieve Connection object
            con = pool_.PopFront();

            if(!con) 
                std::cout<<" INVALID CONNECTION FROM CONNECTION POOL " << std::endl;

            if(con->TestConnection()) { 
                break;
            }
            else {
                std::cout<<" Connection no good deleting, getting another  " << std::endl;
                delete con;
                con = NULL;
            }
        }
    }
    else {
        std::cout<<" CONNECTION MANAGER NOT INITIALIZED ..." << std::endl;
    }
    pool_mtx_.Unlock();
    // Check if connection is still alive ... just do a head request,
    //  - if yes, return
    //  - if not, re-connect then return
    return con;
}

void ConnectionManager::ReclaimConnection(Connection* socket) {
    pool_mtx_.Lock();
    pool_.PushBack(socket);
    pool_mtx_.Unlock();
}

}//namespace
