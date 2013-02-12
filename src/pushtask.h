
#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>
#include <vector>

#include "tenttask.h"
#include "atticpost.h"
#include "chunkpost.h"


class PushTask : public TentTask
{
    int CreateAndSerializeAtticPost( bool pub,
                                     const std::string& filepath,
                                     const std::string& filename,
                                     unsigned int size,
                                     std::string& out);

    int InitChunkPost(ChunkPost& post, FileInfo::ChunkMap& List);
    int InitAtticPost(AtticPost& post,
                        bool pub,
                        const std::string& filepath,
                        const std::string& filename, 
                        const std::string& chunkname,
                        unsigned int size,
                        FileInfo::ChunkMap* pList);

    int PushFile( const std::string& filepath );

    int SendChunkPost( FileInfo* fi, 
                       const std::string& filepath, 
                       const std::string& filename );

    int SendAtticPost( FileInfo* fi, 
                       const std::string& filepath,
                       const std::string& filename);

public:
    PushTask( TentApp* pApp, 
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

    ~PushTask();

    virtual void OnStart() { } 
    virtual void OnPaused() { } 
    virtual void OnFinished() { }

    int GetUploadSpeed();

    void RunTask();

private:
};

#endif

