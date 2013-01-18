
#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>
#include <vector>

#include "tenttask.h"
#include "atticpost.h"
#include "chunkpost.h"

class ChunkInfo;

class PushTask : public TentTask
{
    int CreateAndSerializeAtticPost( bool pub,
                                     const std::string& filepath,
                                     const std::string& filename,
                                     unsigned int size,
                                     std::string& out);

    int InitChunkPost(ChunkPost& post, std::vector<ChunkInfo*>* pList);
    int InitAtticPost(AtticPost& post,
                        bool pub,
                        const std::string& filepath,
                        const std::string& filename, 
                        unsigned int size,
                        std::vector<ChunkInfo*>* pList);

    int PushFile( const std::string& filepath );

public:
    PushTask( TentApp* pApp, 
              FileManager* pFm, 
              ConnectionManager* pCon, 
              CredentialsManager* pCm,
              const AccessToken& at,
              const std::string& entity,
              const std::string& filepath,
              const std::string& tempdir, 
              const std::string& workingdir,
              const std::string& configdir,
              void (*callback)(int, void*));

    ~PushTask();

    void RunTask();

private:
};

#endif

