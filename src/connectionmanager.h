#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_
#pragma once

#include <string>
#define BOOST_NETWORK_ENABLE_HTTPS 
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 

#include "connection.h"
#include "connectionpool.h"
#include "mutexclass.h"

namespace attic {

class ConnectionManager {
public:
    ConnectionManager() {}
    ~ConnectionManager() {}

    int Initialize(const std::string& host_url);
    int Shutdown();

    Connection* RequestConnection();
    void ReclaimConnection(Connection* socket);

private:
    std::string             host_url_;
    boost::asio::io_service io_service_;

    ConnectionPool          open_pool_;
    MutexClass              pool_mtx_;
};

}//namespace
#endif

