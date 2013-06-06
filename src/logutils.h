#ifndef LOGUTILS_H_
#define LOGUTILS_H_
#pragma once

#define DEBUG_OUT 1
#define CERR_OUT 0

#if CERR_OUT
#include <cstdio> 
#endif

#include "event.h"
#include "response.h"

namespace attic { namespace log {
static void LogToStdErr() {
#if CERR_OUT
    freopen( "log.txt", "a", stderr );
#endif
}

static void LogHttpResponse(const std::string& error_ident, const Response& resp) {
    std::ostringstream error;
    error << error_ident << " ";
    error << "Non 200 repsonse" << std::endl;
    error << "Code : " << resp.code << std::endl;
    error << "Headers : " << resp.header.asString() << std::endl;
    error << "Body : " << resp.body << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
#if CERR_OUT
    cerr << error.str() << std::endl;
#endif
}

static void LogException(const std::string& error_ident, std::exception& e) {
    std::ostringstream error;
    error << error_ident << std::endl;
    error << "Exception thrown : " << e.what() << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);

#if CERR_OUT
    cerr << error.str();
#endif
}

static void LogStream(const std::string& error_ident, std::ostringstream& buffer){
    std::ostringstream error;
    error << error_ident << std::endl;
    error << buffer << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
#if CERR_OUT
    cerr << error.str();
#endif
}

static void LogString (const std::string& error_ident, std::string buffer){
    std::ostringstream error;
    error << error_ident << std::endl;
    error << buffer << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
#if CERR_OUT
    cerr << error.str();
#endif
}

// just an alaias for log string
static void ls(const std::string& error_ident, std::string buffer) {
    LogString(error_ident, buffer);
}

static void LogDebugString(const std::string& id, const std::string& buffer) {
#if DEBUG_OUT
    std::ostringstream dbg;
    dbg << id << std::endl;
    dbg << buffer << std::endl;;
    event::RaiseEvent(event::Event::DEBUG_NOTIFY, dbg.str(), NULL);
#endif
}


}}//namespace
#endif

