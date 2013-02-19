#include "multipartconsumer.h"

#include "errorcodes.h"

MultipartConsumer::MultipartConsumer()
    : m_Resolver(m_IO_Service),
      m_Socket(m_IO_Service)
{
}

MultipartConsumer::~MultipartConsumer()
{
}

int MultipartConsumer::ConnectToHost()
{
    int status = ret::A_OK;

    return status;
}

int MultipartConsumer::DisconnectFromHost()
{
    int status = ret::A_OK;

    return status;
}

int MultipartConsumer::PushBodyForm(const std::string& body)
{
    int status = ret::A_OK;

    return status;
}

int MultipartConsumer::PushPartIntoForm(const std::string& buf)
{
    int status = ret::A_OK;

    return status;
}

int SendFooter()
{
    int status = ret::A_OK;

    return status;
}


