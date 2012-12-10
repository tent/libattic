
#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>
#include <deque>

#include "crypto.h"

class FileInfo
{
    void ExtractFileName(const std::string &filepath, std::string &out);
public:
    FileInfo() {} 
    ~FileInfo() {}

    // Init a file
    bool InitializeFile(const std::string &filepath);

    bool IsValid() { return !m_FileName.empty(); }
    
    void SetFileName(const std::string &filename)     { m_FileName = filename; }
    void SetFilePath(const std::string &filepath)     { m_FilePath = filepath; }
    void SetChunkName(const std::string &chunkname)   { m_ChunkName = chunkname; }

    void SetChunkCount(const std::string& count)        { m_ChunkCount = atoi(count.c_str()); }
    void SetChunkCount(const unsigned int unChunkCount) { m_ChunkCount = unChunkCount; }

    void SetFileSize(const std::string& file)           { m_FileSize = atoi(file.c_str()); }
    void SetFileSize(const unsigned int unFileSize)     { m_FileSize = unFileSize; }

    void SetCredentials(const Credentials &tCred)       { m_Credentials = tCred; }

    void SetPostID(const std::string &szID)         { m_PostID = szID; }

    void SetPostVersion(const std::string& version)      { m_PostVersion = atoi(version.c_str()); }
    void SetPostVersion(const unsigned int unVer)   { m_PostVersion = unVer; }

    void SetKey(const std::string &key) { memcpy(&m_Credentials.key, key.c_str(), m_Credentials.GetKeySize() ); }
    void SetIv(const std::string &iv) { memcpy(&m_Credentials.iv, iv.c_str(), m_Credentials.GetIvSize() ); }

    void GetFileName(std::string &out) const    { out = m_FileName; }
    void GetFilePath(std::string &out) const    { out = m_FilePath; }
    void GetPostID(std::string &out) const      { out = m_PostID; }
    void GetChunkName(std::string& out) const   { out = m_ChunkName; }

    void GetKey(std::string &out) const { out.append((const char*)m_Credentials.key, m_Credentials.GetKeySize()); }
    void GetIv(std::string &out) const  { out.append((const char*)m_Credentials.iv, m_Credentials.GetIvSize()); }

    unsigned int GetChunkCount() const      { return m_ChunkCount; }
    unsigned int GetFileSize() const        { return m_FileSize; }
    Credentials GetCredentials() const      { return m_Credentials; }

    unsigned int GetPostVersion() const { return m_PostVersion ; }
private:    
    Credentials     m_Credentials;

    std::string     m_FileName;   // File within directory
    std::string     m_FilePath;   // Directory
    std::string     m_ChunkName;

    std::string     m_PostID; // Id of the post the file is potentially attached to

    unsigned int    m_PostVersion; // Version of the post the file is attached to
    unsigned int    m_ChunkCount;
    unsigned int    m_FileSize;   // Filesize, not compressed
};


class FileInfoFactory
{
public:
    FileInfoFactory();
    ~FileInfoFactory();

    FileInfo* CreateFileInfoObject();

private:
    std::deque<FileInfo*> m_FileList; // List of FileInfo objects created.

};

#endif

