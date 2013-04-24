#include "connectionmanager.h"

#include "errorcodes.h"

namespace attic { 

ConnectionManager* ConnectionManager::instance_ = NULL;
int ConnectionManager::ref_ = 0;

ConnectionManager::ConnectionManager() {
    ref_ = 0;
    initialized_ = false;
}

ConnectionManager::~ConnectionManager() {
    std::cout<<" CONNECTION MANAGER DESTRUCTOR " << std::endl;
}

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
        Shutdown();
    }
}

ConnectionManager* ConnectionManager::GetInstance() {
    if(!instance_)
        instance_ = new ConnectionManager();
    ref_++;
    return instance_;
}

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
    initialized_ = true;
    return status;
}

Connection* ConnectionManager::RequestConnection() {
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
        std::cout<<" CONNECTION MANAGER NOT INITIALIZED " << std::endl;
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
