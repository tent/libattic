#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>

#include "mutexclass.h"
#include "manifest.h"
#include "fileinfo.h"
#include "fileinfofactory.h"
#include "errorcodes.h"
#include "folder.h"

class FileManager : public MutexClass
{
    FileManager(const FileManager &rhs) { }
    FileManager operator=(const FileManager &rhs) { return *this; }


public:
    FileManager();
    FileManager( const std::string &manifestDirectory, 
                 const std::string &workingDirectory);

    ~FileManager();

    bool StartupFileManager();
    bool ShutdownFileManager();

    void InsertToManifest (FileInfo* pFi);
    int RemoveFile(const std::string &filepath);

    FileInfo* CreateFileInfo();

    void GetRelativeFilepath(const std::string& filepath, std::string& out);

    bool DoesFileExist(const std::string& filepath);
    void GetManifestDirectory(std::string &out) const { out = m_ManifestDirectory; }
    void GetWorkingDirectory(std::string &out) const { out = m_WorkingDirectory; }
    void GetTempDirectory(std::string &out) const    { out = m_TempDirectory; }
    int GetAllFileInfo(std::vector<FileInfo>& out);
    FileInfo* GetFileInfo(const std::string &filepath);
    bool GetFolderInfo(const std::string& folderpath, Folder& folder);


    void SetManifestDirectory(const std::string &filepath)       { m_ManifestDirectory = filepath; }
    void SetWorkingDirectory(const std::string &workingDir)     { m_WorkingDirectory = workingDir; }
    void SetTempDirectory(const std::string &tempDir)           { m_TempDirectory = tempDir; }
    void SetFilePostId(const std::string &filepath, const std::string& postid);
    void SetFileChunkPostId(const std::string &filepath, const std::string& postid);
    void SetFolderPostId(const std::string& folderpath, const std::string& postid);

private:
    FileInfoFactory     m_FileInfoFactory;
    Manifest            m_Manifest;

    std::string         m_ManifestDirectory; // Location of manifest
    std::string         m_WorkingDirectory; // Location where original files live.
    std::string         m_TempDirectory;    // Location where file copies will be made and manipulated
};


#endif
