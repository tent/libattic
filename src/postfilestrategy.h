#ifndef POSTFILESTRATEGY_H_
#define POSTFILESTRATEGY_H_
#pragma once

#include "httpstrategy.h"
#include "netlib.h"
#include "credentials.h"

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
    int SendChunk(const std::string& chunk, 
                  const std::string& fileKey,
                  const std::string& boundary,
                  boost::asio::ssl::stream<tcp::socket&>& ssl_sock,
                  const unsigned int count,
                  bool end,
                  FileInfo* pFi);
    int TransformChunk(const std::string& chunk, 
                       const std::string& fileKey,
                       std::string& finalizedOut, 
                       std::string& nameOut, 
                       FileInfo* pFi);
    void UpdateFileInfo(const Credentials& fileCred, 
                        const std::string& filepath, 
                        const std::string& chunkpostid,
                        FileInfo* fi);
    FileInfo* RetrieveFileInfo(const std::string& filepath);
public:
    PostFileStrategy();
    ~PostFileStrategy();

    int Execute(FileManager* pFileManager,
                CredentialsManager* pCredentialsManager,
                const std::string& entityApiRoot, 
                const std::string& filepath, 
                Response& out);
};
#endif

