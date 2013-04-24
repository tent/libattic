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
    friend class ConnectionHandler;
    ConnectionManager();
    ConnectionManager(const ConnectionManager& rhs) {}
    ConnectionManager operator=(const ConnectionManager& rhs) { return *this; }

    static ConnectionManager* GetInstance();
    Connection* RequestConnection();
    void ReclaimConnection(Connection* socket);
    int Shutdown();
    void Release();
public:
    ~ConnectionManager();
    int Initialize(const std::string& host_url);

private:
    std::string             host_url_;
    boost::asio::io_service io_service_;

    ConnectionPool          pool_;
    MutexClass              pool_mtx_;

    static ConnectionManager* instance_;
    static int ref_;
    bool initialized_;
};

}//namespace
#endif

