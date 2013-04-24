#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_
#pragma once

#include <string>
#include <deque>
#include "connection.h"
namespace attic { 

class ConnectionPool {
    typedef std::deque<Connection*> ConnectionQueue;
public:
    ConnectionPool();
    ~ConnectionPool();

    void PushBack(Connection* con);
    Connection* PopFront();

    void ExtendPool(boost::asio::io_service* io_service, 
                    const std::string& host, 
                    const int stride = 3);

    bool empty() { return pool_.empty(); }
private:
    ConnectionQueue pool_;
};

}//namespace
#endif

