#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "manifest.h"
#include "chunker.h"
#include "fileinfo.h"
#include "compressor.h"
#include "crypto.h"
#include "errorcodes.h"

#include "taskarbiter.h"

class FileManager : public MutexClass
{
    FileInfo* CreateFileInfo();
    bool ReadInHeader(std::string &h);
    bool ReadInEntry(std::string &e);

    void ConstructOutboundPath( const std::string &workingDir, 
                                const std::string &filename, 
                                const std::string &postfix,
                                std::string &outboundPath,
                                bool bStripFileType);
    
    void GenerateCompressionPath(FileInfo* fi, std::string &outpath);
    void GenerateCryptoPath(FileInfo* fi, std::string &outpath);

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
   
    ret::eCode IndexFile(const std::string &filepath, const bool insert);
    ret::eCode RemoveFile(const std::string &filename);

    ret::eCode ConstructFile(std::string &filename);

    bool FindFileInManifest(const std::string &filename);   // File exists in manifest
    bool FileExists(std::string& filepath);               // File exists on disc

    FileInfo* GetFileInfo(const std::string &filename);

    FileInfo* CreateFileInfo( const std::string &filename,
                              const std::string &filepath,
                              const std::string &chunkName,
                              const std::string &chunkCount,
                              const std::string &fileSize,
                              const std::string &postId,
                              const std::string &postVersion,
                              const std::string &key,
                              const std::string &iv);

    void InsertToManifest (FileInfo* fi) { if(fi) m_Manifest.InsertFileInfo(fi); }
    
    unsigned int GetManifestVersion() const          { return m_Manifest.GetVersionNumber(); }
    void GetManifestPostID(std::string &out) const   { m_Manifest.GetPostID(out); }
    void GetManifestFilePath(std::string &out) const { out = m_ManifestFilePath; }
    void GetWorkingDirectory(std::string &out) const { out = m_WorkingDirectory; }
    void GetTempDirectory(std::string &out) const    { out = m_TempDirectory; }
    unsigned int GetFileStride() const               { return m_FileStride; }

    void SetManifestFilePath(const std::string &filepath)     { m_ManifestFilePath = filepath; }
    void SetManifestPostID(const std::string &id)               { m_Manifest.SetPostID(id); }
    void SetWorkingDirectory(const std::string &workingDir)     { m_WorkingDirectory = workingDir; }
    void SetTempDirectory(const std::string &tempDir)           { m_TempDirectory = tempDir; }
    void SetFileStride(unsigned int uFileStride )               { m_FileStride = uFileStride; }
    void SetFilePostId(const std::string &filename, const std::string postid);

private:
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
