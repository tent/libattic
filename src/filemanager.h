#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "mutexclass.h"
#include "manifest.h"
#include "chunker.h"
#include "fileinfo.h"
#include "fileinfofactory.h"
#include "compressor.h"
#include "crypto.h"
#include "errorcodes.h"

#include "taskarbiter.h"
#include "masterkey.h"

class FileManager : public MutexClass
{
    FileManager(const FileManager &rhs) { }
    FileManager operator=(const FileManager &rhs) { return *this; }
public:
    FileManager();
    FileManager( const std::string &manifestpath, 
                 const std::string &workingDirectory, 
                 unsigned int uFileStride = 400000);

    ~FileManager();

    bool StartupFileManager();
    bool ShutdownFileManager();

    int RemoveFile(const std::string &filepath);

    FileInfo* GetFileInfo(const std::string &filepath);

    int GetAllFileInfo(std::vector<FileInfo>& out);


    FileInfo* CreateFileInfo();

    FileInfo* CreateFileInfo( const std::string &filename,
                              const std::string &filepath,
                              const std::string &chunkName,
                              const std::string &chunkCount,
                              const std::string &fileSize,
                              const std::string &postId,
                              const std::string &postVersion,
                              const std::string &key,
                              const std::string &iv);

    FileInfo* CreateFileInfo( const std::string &filename,
                              const std::string &filepath,
                              const std::string &chunkName,
                              const std::string &chunkCount,
                              const std::string &fileSize,
                              const std::string &postId,
                              const std::string &postVersion,
                              unsigned char *key, // byte
                              unsigned char *iv); // byte

    void InsertToManifest (const FileInfo* pFi) { 
        Lock();
        if(pFi) m_Manifest.InsertFileInfo(pFi); 
        Unlock();
    }
    
    unsigned int GetManifestVersion() const          { return m_Manifest.GetVersionNumber(); }

    void GetManifestFilePath(std::string &out) const { out = m_ManifestFilePath; }
    void GetWorkingDirectory(std::string &out) const { out = m_WorkingDirectory; }
    void GetTempDirectory(std::string &out) const    { out = m_TempDirectory; }
    unsigned int GetFileStride() const               { return m_FileStride; }

    void SetManifestFilePath(const std::string &filepath)       { m_ManifestFilePath = filepath; }
    void SetManifestPostID(const std::string &id)               { m_Manifest.SetPostID(id); } // Depricated
    void SetWorkingDirectory(const std::string &workingDir)     { m_WorkingDirectory = workingDir; }
    void SetTempDirectory(const std::string &tempDir)           { m_TempDirectory = tempDir; }
    void SetFileStride(unsigned int uFileStride )               { m_FileStride = uFileStride; }
    void SetFilePostId(const std::string &filepath, const std::string& postid);
    void SetFileChunkPostId(const std::string &filepath, const std::string& postid);
    void SetMasterKey(const MasterKey& mk)                      { m_MasterKey = mk; } 

private:
    MasterKey           m_MasterKey;

    FileInfoFactory     m_FileInfoFactory;
    Manifest            m_Manifest;
    Chunker             m_Chunker;
    Crypto              m_Crypto;
    Compressor          m_Compressor;

    std::ifstream       m_ifStream;
    std::ofstream       m_ofStream;

    std::string         m_ManifestFilePath; // Location of manifest
    std::string         m_WorkingDirectory; // Location where original files live.
    std::string         m_TempDirectory;    // Location where file copies will be made and manipulated
                                            // compression, chunking, cryptio, etc ...
                                            
    unsigned int        m_FileStride;       // Generic file stride to be used by chunker,  
                                            // compressor, and crypto
};


#endif
