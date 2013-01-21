#include "credentials.h"

#include "errorcodes.h"

Credentials::Credentials()
{

}

Credentials::~Credentials()
{

}

int Credentials::SetKey(const std::string& key) 
{ 
    int status = ret::A_OK;
    if(key.size() == GetKeySize())
    {
        memcpy(m_Key, reinterpret_cast<const unsigned char*>(key.c_str()), GetKeySize());
    }
    else
    {
        status = ret::A_FAIL_KEYSIZE_MISMATCH;
    }

    return status;
}

int Credentials::SetIv(const std::string& iv) 
{ 
    int status = ret::A_OK;
    if(iv.size() == GetIvSize())
    {
        memcpy(m_Iv, reinterpret_cast<const unsigned char*>(iv.c_str()), GetIvSize());
    }
    else
    {
        status = ret::A_FAIL_IVSIZE_MISMATCH;
    }

    return status;
}


