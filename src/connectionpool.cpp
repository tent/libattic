#include "connectionpool.h"

#include <iostream>

namespace attic { 

ConnectionPool::ConnectionPool() {}
ConnectionPool::~ConnectionPool() {
    ConnectionQueue::iterator itr = pool_.begin();
    for(;itr!=pool_.end(); itr++) { 
        Connection* hold = *itr;
        (*itr) = NULL;
        if(hold) {
            std::cout<<" deleting connection ... " << std::endl;
            delete hold;
            hold = NULL;
        }
    }
}

void ConnectionPool::PushBack(Connection* con) {
    if(con) {
        pool_.push_back(con);
    }
}

Connection* ConnectionPool::PopFront() {
    Connection* con = pool_.front();
    pool_.pop_front();
    return con;
}

void ConnectionPool::ExtendPool(boost::asio::io_service* io_service, 
                                const std::string& host, 
                                const int stride) {
    std::cout<<" EXTENDING POOL " << std::endl;
    for(int i=0; i < stride; i++) { 
        Connection* hold = new Connection(io_service);
        hold->Initialize(host);
        pool_.push_back(hold);
    }
}


}// namespace
