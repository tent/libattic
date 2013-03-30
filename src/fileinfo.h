#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>
#include <map>
#include <vector>

#include "crypto.h"
#include "chunkinfo.h"

class FileInfo {
    void ExtractFilename(const std::string &filepath, std::string &out);
public:
    typedef std::map<std::string, ChunkInfo> ChunkMap;

    FileInfo( const std::string& filename,
              const std::string& filepath,
              const std::string& postid,
              const std::string& chunkpostid);

    FileInfo();  
    ~FileInfo(); 

    // Init a file
    bool InitializeFile(const std::string &filepath);

    bool IsValid() { return !m_Filename.empty(); }
    
    void SetFilename(const std::string &filename)     { m_Filename = filename; }
    void SetFilepath(const std::string &filepath)     { m_Filepath = filepath; }

    void SetChunkName(const std::string &chunkname)     { m_ChunkName = chunkname; }
    void SetChunkCount(const std::string& count)        { m_ChunkCount = atoi(count.c_str()); }
    void SetChunkCount(const unsigned int unChunkCount) { m_ChunkCount = unChunkCount; }

    void SetFileSize(const std::string& file)           { m_FileSize = atoi(file.c_str()); }
    void SetFileSize(const unsigned int unFileSize)     { m_FileSize = unFileSize; }

    void SetPostID(const std::string &szID)         { m_PostID = szID; }
    void SetChunkPostID(const std::string &szID)    { m_ChunkPostID = szID; }
    void SetPostVersion(const std::string& version) { m_PostVersion = atoi(version.c_str()); }
    void SetPostVersion(const unsigned int unVer)   { m_PostVersion = unVer; } // Depricated 

    void SetFileCredentials(const Credentials& cred)    { m_FileCredentials = cred; }
    void SetFileKey(const std::string &key)             { m_FileCredentials.set_key(key); }
    void SetEncryptedKey(const std::string& key)        { m_EncryptedKey = key; } 
    void SetIv(const std::string &iv)                   { m_FileCredentials.set_iv(iv); }
    void SetDeleted(const std::string& deleted)         { m_Deleted = atoi(deleted.c_str()); }
    void SetDeleted(const int deleted)                 { m_Deleted = deleted; }

    void GetFilename(std::string &out) const    { out = m_Filename; }
    void GetFilepath(std::string &out) const    { out = m_Filepath; }
    void GetPostID(std::string &out) const      { out = m_PostID; }
    void GetChunkPostID(std::string &out) const { out = m_ChunkPostID; }
    void GetChunkName(std::string& out) const   { out = m_ChunkName; }

    void GetFileCredentials(Credentials& cred) const    { cred = m_FileCredentials; }
    void GetFileKey(std::string &out) const             { out = m_FileCredentials.key(); }
    void GetEncryptedKey(std::string& out) const        { out = m_EncryptedKey; }
    void GetIv(std::string &out) const                  { out = m_FileCredentials.iv(); }
    int GetDeleted() const                              { return m_Deleted; }

    unsigned int GetChunkCount() const      { return m_ChunkCount; }
    unsigned int GetFileSize() const        { return m_FileSize; }

    Credentials GetCredentialsCopy() const  { return m_FileCredentials; }

    unsigned int GetPostVersion() const { return m_PostVersion ; }

    int PushChunkBack(ChunkInfo& Chunk);
    ChunkInfo* GetChunkInfo(const std::string& chunkname);

    ChunkMap* GetChunkInfoList() { return &m_Chunks; }

    void GetSerializedChunkData(std::string& out) const;
    bool LoadSerializedChunkData(const std::string& data);
    bool LoadSerializedChunkPost(const std::string& data);

    bool HasEncryptedKey();

private:    
    ChunkMap        m_Chunks;
    Credentials     m_FileCredentials; // File Specific credentials 
                                       // Key is used to encrypt chunks, an iv is specific to chunk
                                       // Iv is used to encrypt this key
    std::string     m_EncryptedKey;

    std::string     m_Filename;   // File within directory
    std::string     m_Filepath;   // Directory
    std::string     m_ChunkName;                                                    // Depricated

    std::string     m_PostID; // Id of the post the file is potentially attached to
    std::string     m_ChunkPostID; // Id of the chunk post

    unsigned int    m_PostVersion; // Version of the post the file is attached to
    unsigned int    m_ChunkCount; // depricated
    unsigned int    m_FileSize;   // Filesize, not compressed
    int    m_Deleted; // Is the file deleted? // soft delete
};

#endif

