
#ifndef FILEINFO_H_
#define FILEINFO_H_
#pragma once

#include <string>
#include <deque>

#include "crypto.h"

class FileInfo
{
    std::string ExtractFileName(const std::string &filepath);
public:
    FileInfo();
    ~FileInfo();

    // Init a file
    bool InitializeFile(const std::string &filepath);
    
    void SetFileName(const std::string &filename)     { m_FileName = filename; }
    void SetFilePath(const std::string &filepath)     { m_FilePath = filepath; }
    void SetChunkName(const std::string &chunkname)   { m_ChunkName = chunkname; }
    void SetChunkCount(const unsigned int unChunkCount) { m_ChunkCount = unChunkCount; }
    void SetFileSize(const unsigned int unFileSize)     { m_FileSize = unFileSize; }
    void SetCredentials(const Credentials &tCred)       { m_Credentials = tCred; }

    void SetPostID(const std::string &szID)         { m_PostID = szID; }
    void SetPostVersion(const unsigned int unVer)   { m_PostVersion = unVer; }
    
    std::string GetFileName() const         { return m_FileName; }
    std::string GetFilePath() const         { return m_FilePath; }
    std::string GetChunkName() const        { return m_ChunkName; }
    unsigned int GetChunkCount() const      { return m_ChunkCount; }
    unsigned int GetFileSize() const        { return m_FileSize; }
    Credentials GetCredentials() const      { return m_Credentials; }

    std::string GetPostID() const       { return m_PostID; }
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

