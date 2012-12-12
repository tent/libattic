
#ifndef DELETETASK_H_
#define DELETETASK_H_
#pragma once

#include "tenttask.h"

class DeleteTask : public TentTask
{
    int DeleteFile(const std::string& filename);
    int DeletePost(const std::string& szPostID);

public:
    DeleteTask( TentApp* pApp, 
                FileManager* pFm, 
                ConnectionManager* pCon, 
                const AccessToken& at,
                const std::string& entity,
                const std::string& filepath,
                const std::string& tempdir, 
                const std::string& workingdir,
                const std::string& configdir,
                void (*callback)(int, void*));

    ~DeleteTask();

    void RunTask();


};

#endif

