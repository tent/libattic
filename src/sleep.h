#ifndef SLEEP_H_
#define SLEEP_H_
#pragma once

#include <boost/thread/thread.hpp>

namespace attic { namespace sleep {

static void sleep_milliseconds(unsigned int milliseconds) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(milliseconds));  
}

static void mil(unsigned int milliseconds) {
    sleep_milliseconds(milliseconds);
}

static void sleep_seconds(unsigned int seconds) {
    boost::this_thread::sleep_for(boost::chrono::seconds(seconds));  
}

static void sec(unsigned int seconds) {
    sleep_seconds(seconds);
}

}}// namespace
#endif

