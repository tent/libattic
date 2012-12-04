
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

    void SetKey(const std::string &key) { m_Key = key; }
    void SetIv(const std::string &iv) { m_Iv = iv; }
    
    void GetFileName(std::string &out) const    { out = m_FileName; }
    void GetFilePath(std::string &out) const    { out = m_FilePath; }
    void GetPostID(std::string &out) const      { out = m_PostID; }
    void GetChunkName(std::string& out) const   { out = m_ChunkName; }
    void GetKey(std::string &out) const         { out = m_Key; }
    void GetIv(std::string &out) const          { out = m_Iv; }

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

    std::string     m_Key;
    std::string     m_Iv;

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

