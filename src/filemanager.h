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
#include "filequeue.h"

namespace attic { 

class FileManager : public MutexClass {
    FileManager(const FileManager &rhs) {}
    FileManager operator=(const FileManager &rhs) { return *this; }


    void ExtractRelativePaths(const FileInfo* pFi, 
                              std::string& relative_out, 
                              std::string& parent_relative_out);
    bool IsPathRelative(const std::string& filepath);

    bool AttemptToGetRelativePath(const std::string& filepath, std::string& out);
public:
    FileManager();
    ~FileManager();

    int Initialize(const std::string& manifestDirectory, 
                   const std::string& workingDirectory,
                   const std::string& tempDirectory);
    int Shutdown();

    void InsertToManifest (FileInfo* pFi);
    int RemoveFile(const std::string &filepath);
    void GetRelativePath(const std::string& filepath, std::string& relative_out);

    int RenameFile(const std::string& old_filepath, const std::string& new_filename);

    FileInfo* CreateFileInfo();
    void GetCanonicalFilepath(const std::string& relativepath, std::string& out);

    bool DoesFileExist(const std::string& filepath);
    int GetAllFileInfo(std::vector<FileInfo>& out);
    FileInfo* GetFileInfo(const std::string &filepath);

    void SetFileVersion(const std::string& filepath, const std::string& version);
    void SetFileDeleted(const std::string& filepath, const bool del = true);
    void SetFilePostId(const std::string &filepath, const std::string& postid);
    void SetFileChunkPostId(const std::string &filepath, const std::string& postid);

    // Folder
    bool GetFolderEntry(const std::string& folderpath, Folder& folder);
    bool CreateFolderEntry(const std::string& folderpath, 
                           const std::string& folder_post_id);
    bool SetFolderPostId(const std::string& folderpath, const std::string& post_id);

    // File Queue
    bool LockFile(const std::string& filepath);
    bool UnlockFile(const std::string& filepath);
    bool IsFileLocked(const std::string& filepath);

    // Accessor / Mutator
    const std::string& manifest_directory() const   { return manifest_directory_; }
    const std::string& working_directory() const    { return working_directory_; }
    const std::string& temp_directory() const       { return temp_directory_; }

    void set_manifest_directory(const std::string &filepath)      { manifest_directory_ = filepath; }
    void set_working_directory(const std::string &workingDir)     { working_directory_ = workingDir; }
    void set_temp_directory(const std::string &tempDir)           { temp_directory_ = tempDir; }
private:
    CentralFileQueue    file_queue_;
    FileInfoFactory     file_info_factory_;
    Manifest            manifest_;

    std::string         manifest_directory_; // Location of manifest
    std::string         working_directory_; // Location where original files live.
    std::string         temp_directory_;    // Location where file copies will be made and manipulated
};

} //namespace
#endif
