#ifndef DELETEALLPOSTSTASK_H_
#define DELETEALLPOSTSTASK_H_
#pragma once

#include "tenttask.h"

class DeleteAllPostsTask : public TentTask
{
    int DeletePost(const std::string& postId);
public:
    DeleteAllPostsTask( TentApp* pApp, 
                        FileManager* pFm, 
                        CredentialsManager* pCm,
                        TaskArbiter* pTa,
                        TaskFactory* pTf,
                        const AccessToken& at,
                        const Entity& entity,
                        const std::string& filepath,
                        const std::string& tempdir,
                        const std::string& workingdir,
                        const std::string& configdir,
                        void (*callback)(int, void*));

    ~DeleteAllPostsTask();
    
    virtual void OnStart();
    virtual void OnPaused();
    virtual void OnFinished();

    void RunTask();


};


#endif

