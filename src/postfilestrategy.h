#ifndef POSTFILESTRATEGY_H_
#define POSTFILESTRATEGY_H_
#pragma once

#include "httpstrategy.h"
#include "netlib.h"
#include "credentials.h"
#include "connection.h"
#include "chunkpost.h"

namespace attic {

class FileInfo;
class FileManager;

class PostFileStrategy : public HttpStrategyInterface {
    int DetermineChunkPostRequest(FileInfo* fi, 
                                  Credentials& credOut, 
                                  std::string& requestTypeOut,
                                  std::string& urlOut,
                                  std::string& postidOut);

    int ProcessFile(const std::string& requestType,
                    const std::string& url,
                    const std::string& filepath,
                    const Credentials& fileCredentials,
                    FileInfo* pFi,
                    Response& resp);

    int ProcessChunk(const std::string& chunk, 
                     const std::string& file_key, 
                     const std::string& request_boundary,
                     const unsigned int chunk_count,
                     Connection& socket,
                     FileInfo* fi,
                     std::string& name_out);

    int PrepareChunkPost(std::vector<std::string>& chunk_list, 
                         ChunkPost& prev_post,
                         FileInfo* fi, 
                         ChunkPost& out);

    int SendChunk(const std::string& chunk, 
                  const std::string& chunk_name,
                  const std::string& boundary,
                  Connection& socket,
                  const unsigned int count,
                  bool end);

    int WriteToSocket(Connection& socket,
                      boost::asio::streambuf& buffer);

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

    int UpdateChunkPostMetadata(FileInfo* fi, 
                                const Response& resp, 
                                const Credentials& file_cred);

public:
    PostFileStrategy();
    ~PostFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager);
};

}//namespace
#endif

