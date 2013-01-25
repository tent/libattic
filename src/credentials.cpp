#include "credentials.h"

#include "errorcodes.h"

Credentials::Credentials()
{
    memset(m_Key, 0, CryptoPP::AES::MAX_KEYLENGTH+1);
    memset(m_Iv, 0, CryptoPP::AES::BLOCKSIZE+1);

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
        //memcpy(m_Key, reinterpret_cast<const unsigned char*>(key.c_str()), key.size());

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
        //memcpy(m_Iv, reinterpret_cast<const unsigned char*>(iv.c_str()), iv.size());
    }
    else
    {
        status = ret::A_FAIL_IVSIZE_MISMATCH;
    }

    return status;
}


