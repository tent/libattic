

#ifndef CRYPTOTASK_H_
#define CRYPTOTASK_H_
#pragma once

#include "task.h"
#include "crypto.h"

class CryptoTask : public Task
{
public:
    CryptoTask() {} 
    virtual ~CryptoTask() {}

    /*
    virtual void RunTask() {}
    */

    Crypto* GetCrypto() { return &m_Crypto; }

protected:
    // TODO :: this is also in tent task, perhaps abstract this.
    void Callback(int code, void* p)
    {
        if(mCallback)
            mCallback(code, p); 
    }

private:
    Crypto m_Crypto;

    void (*mCallback)(int, void*);
};

#endif

