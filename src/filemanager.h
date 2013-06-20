#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_
#pragma once

#include <fstream>
#include <string>
#include <map>
#include <deque>

#include "mutexclass.h"
#include "manifest.h"
#include "fileinfo.h"
#include "errorcodes.h"
#include "folder.h"
#include "filequeue.h"
#include "folderpost.h"

namespace attic { 

class FileManager {
    FileManager(const FileManager &rhs) {}
    FileManager operator=(const FileManager &rhs) { return *this; }


    void ExtractRelativePaths(const FileInfo* pFi, 
                              std::string& relative_out, 
                              std::string& parent_relative_out);

    bool IsPathAliased(const std::string& filepath);
public:
    FileManager();
    ~FileManager();

    int Initialize(const std::string& manifestDirectory, 
                   const std::string& workingDirectory,
                   const std::string& tempDirectory);
    int Shutdown();

    bool InsertToManifest (FileInfo* pFi);
    int RemoveFile(const std::string &filepath);

    int RenameFile(const std::string& old_filepath, 
                   const std::string& new_filepath);

    bool GetCanonicalPath(const std::string& relativepath, std::string& out);
    bool GetAliasedPath(const std::string& filepath, std::string& out);

    bool MarkFilesInFolderDeleted(const std::string& folder_id);
    bool MarkFilesInFolderDeleted(const Folder& folder);

    bool DoesFileExist(const std::string& filepath);
    int GetAllFileInfo(std::deque<FileInfo>& out);

    int GetAllFileInfoForFolder(const std::string& folderid, std::deque<FileInfo>& out);
    int GetAllFoldersForFolder(const std::string& folderid, std::deque<Folder>& out);

    bool GetFileInfo(const std::string& filepath, FileInfo& out);
    bool GetFileInfoByPostId(const std::string& post_id, FileInfo& out);

    bool SetFileVersion(const std::string& filepath, const std::string& version);
    bool SetFileDeleted(const std::string& filepath, const bool del = true);
    bool SetFilePostId(const std::string& filepath, const std::string& postid);
    bool SetFileChunks(const std::string& filepath, FileInfo::ChunkMap& map);
    bool SetFileChunkCount(const std::string& filepath, const std::string& count);
    bool SetFileFolderPostId(const std::string& filepath, const std::string& postid);
    bool SetNewFilepath(const std::string& old_filepath, const std::string& new_filepath);

    // Folder
    bool CreateFolderEntry(const std::string& foldername, 
                           const std::string& folder_post_id,
                           const std::string& parent_post_id,
                           Folder& out);
    bool RemoveFolderEntry(const std::string& foldername, const std::string& parent_post_id);
    bool DoesFolderExist(const std::string& foldername, const std::string& parent_post_id);
    bool DoesFolderExistById(const std::string& post_id);


    bool GetFolderEntryByPostId(const std::string& post_id, Folder& folder);
    bool GetFolderEntry(const std::string& foldername, 
                        const std::string& parent_post_id,
                        Folder& out);
    // TODO * reevaluate the need for this method
    bool GetFolderEntry(const std::string& folderpath, Folder& folder);
    bool GetFolderPostId(const std::string& folderpath, std::string& id_out);

    bool GetFolderParentId(const std::string& post_id, std::string& out);


    bool SetFoldername(const std::string& post_id, const std::string& foldername);
    bool SetFolderPostId(const std::string& post_id, const std::string& new_post_id);
    bool SetFolderParentPostId(const std::string& post_id, const std::string& parent_post_id);
    bool SetFolderDeleted(const std::string& post_id, bool del);


    bool UpdateFolderEntry(const std::string& foldername, const std::string& post_id);

    bool IsFolderDeleted(const std::string& post_id);
    bool IsRootDirectory(const std::string& post_id);
    bool ConstructFolderpath(const std::string& folder_post_id, std::string& path_out);

    // File Queue
    bool LockFile(const std::string& filepath);
    bool UnlockFile(const std::string& filepath);
    bool IsFileLocked(const std::string& filepath);

    // Config methods /  Working Directories 
    bool HasConfigValue(const std::string& value);
    bool HasConfigKey(const std::string& key);
    bool GetConfigEntry(const std::string& key, ConfigEntry& out);
    bool PushConfigValue(const std::string& type, const std::string& key, const std::string& value);

    bool AddWorkingDirectory(const std::string& directory_alias,
                             const std::string& directory_path, 
                             const std::string& post_id);
    bool IsDirectoryLinked(const std::string& directory_path);
    bool LinkWorkingDirectory(const std::string& config_key, const std::string& directory_path);
    bool UnlinkWorkingDirectory(const std::string& directory_path);
    bool RemoveWorkingDirectory(const std::string& directory_path);
    bool RetrieveAllConfigEntries(std::deque<ConfigEntry>& out);
    bool RetrieveAllConfigEntriesOfType(const std::string& type, std::deque<ConfigEntry>& out);

    bool LoadWorkingDirectories();
    // Get the aliased working dir for an absolute filepath
    bool FindAssociatedWorkingDirectory(const std::string& filepath, 
                                        std::string& dir_out, 
                                        std::string& post_id);
    // Get the canonical path mapped for a specific directory alias
    bool FindAssociatedCanonicalDirectory(const std::string& aliased_path,
                                          std::string& dir_out, 
                                          std::string& post_id);


    // Accessor / Mutator
    const std::string& manifest_directory() const   { return manifest_directory_; }
    const std::string& working_directory() const    { return working_directory_; }
    const std::string& temp_directory() const       { return temp_directory_; }

    void set_manifest_directory(const std::string &filepath)      { manifest_directory_ = filepath; }
    void set_working_directory(const std::string &workingDir)     { working_directory_ = workingDir; }
    void set_temp_directory(const std::string &tempDir)           { temp_directory_ = tempDir; }

private:
    CentralFileQueue    file_queue_;
    MutexClass          manifest_mtx_;
    Manifest            manifest_;

    // Config table
    // key : alias, value : id, state : path
    // This mapping
    // key : alias (key), value : mapping (state)
    MutexClass working_mtx_;
    std::map<std::string, std::string>  working_directories_;

    std::string         manifest_directory_; // Location of manifest
    std::string         working_directory_; // Location where original files live.
    std::string         temp_directory_;    // Location where file copies will be made and manipulated
};

} //namespace
#endif
