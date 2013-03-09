#ifndef PUSHTASK_H_
#define PUSHTASK_H_
#pragma once

#include <string>
#include <vector>

#include "tenttask.h"
#include "atticpost.h"
#include "chunkpost.h"
#include "response.h"
#include "netlib.h"


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

    int SendAtticPost( FileInfo* fi, const std::string& filepath );
    int SendFolderPost(const FileInfo* fi); 
    int ProcessFile( const std::string& requestType,
                     const std::string& url,
                     const std::string& filepath,
                     const Credentials& fileCredentials,
                     FileInfo* pFi,
                     Response& resp);
    
    int TransformChunk( const std::string& chunk, 
                        const std::string& fileKey,
                        std::string& finalizedOut, 
                        std::string& nameOut, 
                        FileInfo* pFi);

    int SendChunk( const std::string& chunk, 
                   const std::string& fileKey,
                   const std::string& boundary,
                   boost::asio::ssl::stream<tcp::socket&>& ssl_sock,
                   const unsigned int count,
                   bool end,
                   FileInfo* pFi);


    FileInfo* RetrieveFileInfo(const std::string& filepath);


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

