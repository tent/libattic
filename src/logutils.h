#ifndef LOGUTILS_H_
#define LOGUTILS_H_
#pragma once

#include "event.h"
#include "response.h"

namespace attic { namespace log {

static void LogHttpResponse(const std::string& error_ident, const Response& resp) {
    std::ostringstream error;
    error << error_ident << " ";
    error << "Non 200 repsonse" << std::endl;
    error << "Code : " << resp.code << std::endl;
    error << "Headers : " << resp.header.asString() << std::endl;
    error << "Body : " << resp.body << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
}

static void LogException(const std::string& error_ident, std::exception& e) {
    std::ostringstream error;
    error << error_ident << std::endl;
    error << "Exception thrown : " << e.what() << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
}

static void LogString(const std::string& error_ident, std::string& buffer) {
    std::ostringstream error;
    error << error_ident << std::endl;
    error << buffer << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
}


}}//namespace
#endif

