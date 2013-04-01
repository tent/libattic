#ifndef LOGUTILS_H_
#define LOGUTILS_H_
#pragma once

#include "event.h"
#include "response.h"

namespace attic { namespace log {

static void LogHttpResponse(const std::string& error_ident, const Response& resp) {
    std::ostringstream error;
    errpr << error_ident << " ";
    error << "Non 200 repsonse" << std::endl;
    error << "Code : \n" << response.code << std::endl;
    error << "Headers : \n" << response.headers.asString() << std::endl;
    error << "Body : \n" << response.body << std::endl;
    event::RaiseEvent(event::Event::ERROR_NOTIFY, error.str(), NULL);
}


}}//namespace
#endif

