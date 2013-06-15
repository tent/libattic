#include "connectionmanager.h"

#include "errorcodes.h"
#include "netlib.h"

namespace attic { 

ConnectionManager* ConnectionManager::instance_ = NULL;
bool ConnectionManager::initialized_ = false;
int ConnectionManager::ref_ = 0;

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager() {
    std::cout<<" CONNECTION MANAGER DESTRUCTOR " << std::endl;
}

int ConnectionManager::Shutdown() {
    std::cout<<" Connection Manager Shutting Down " << std::endl;
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
    std::cout<< "attempting to init " << host_url << std::endl;
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

    std::cout<<" init status : " << status << std::endl;
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
                std::cout<<" connection is good " << std::endl;
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
    std::cout<<" returning con " << std::endl;
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
