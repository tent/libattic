#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_
#pragma once

#include <deque>
#include "connection.h"
namespace attic { 

class ConnectionPool {
public:
    ConnectionPool();
    ~ConnectionPool();

    void PushBack(Connection* con);
    Connection* PopFront();

private:
    std::deque<Connection*> pool_;
};

}//namespace
#endif

