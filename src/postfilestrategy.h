#ifndef POSTFILESTRATEGY_H_
#define POSTFILESTRATEGY_H_
#pragma once

#include <map>
#include <vector>
#include <string>

#include "httpstrategy.h"
#include "netlib.h"
#include "credentials.h"
#include "connection.h"
#include "chunkpost.h"
#include "fileinfo.h"

namespace attic {

class FileManager;

class PostFileStrategy : public HttpStrategyInterface {
public:
    typedef std::map<unsigned int, ChunkPost> ChunkPostList; // key, chunk group
private:
    int TransformChunk(const std::string& chunk, 
                       const std::string& fileKey,
                       std::string& finalizedOut, 
                       std::string& nameOut, 
                       ChunkInfo& out);

    void UpdateFileInfo(const Credentials& fileCred, 
                        const std::string& filepath, 
                        const std::string& chunkpostid,
                        const std::string& post_version,
                        FileInfo* fi);

    FileInfo* RetrieveFileInfo(const std::string& filepath);

    int InitializeFileMetaData(FileInfo* fi, const std::string& filepath, std::string& post_id_out);

    int RetrieveChunkPosts(const std::string& entity,
                           const std::string& post_id,
                           ChunkPostList& out);

    int ChunkFile(const std::string& filepath,
                  const Credentials& file_credentials,
                  const std::string& file_meta_post_id,
                  ChunkPostList& chunk_list,
                  FileInfo::ChunkMap& chunk_map);

    void ExtractChunkInfo(ChunkPostList& list,
                          FileInfo::ChunkMap& out);
public:
    PostFileStrategy();
    ~PostFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

