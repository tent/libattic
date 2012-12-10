

#ifndef PULLTASK_H_
#define PULLTASK_H_
#pragma once

#include "task.h"

class PullTask: public Task
{
public:
    PullTask();
    ~PullTask();

    virtual void RunTask();


};

#endif

