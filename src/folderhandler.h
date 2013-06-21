#ifndef FOLDERHANDLER_H_
#define FOLDERHANDLER_H_
#pragma once

#include <string>
#include <deque>
#include "folder.h"
#include "folderpost.h"
#include "filemanager.h"

#include "accesstoken.h"


namespace attic {

class FolderHandler {
    int RetrieveSubFolders(Folder& folder, std::deque<Folder>& out);
    int RetrieveFilesInFolder(Folder& folder, std::deque<FileInfo>& out);
    int RetrieveAllFilesAndFoldersInFolder(Folder& folder, 
                                           std::deque<FileInfo>& file_out,
                                           std::deque<Folder>& folder_out);
    void SeparatePath(const std::string& full_path, std::deque<std::string>& names);

    bool CreateDirectoryTree(FolderPost& fp);
public:
    FolderHandler(FileManager* fm);
    ~FolderHandler();

    bool ValidateFolder(FolderPost& fp);

    // Validate an absolute folderpath
    bool ValidateFolderPath(const std::string& folderpath, 
                            const std::string& posts_feed,
                            const std::string& post_path,
                            const AccessToken& at);

    // Validates that the posts exist within the local cache, does not update them
    bool ValidateFolderTree(const std::string& folder_post_id,
                            const std::string& post_path,
                            const AccessToken& at);

    bool RetrieveFolders(const std::string& folderpath, 
                         const std::string& working_directory,
                         std::deque<std::string>& out);
 
    bool InsertFolder(const Folder& folder);
    bool InsertFolder(const FolderPost& fp);
    bool SetFolderPostId(Folder& folder, const std::string& post_id);
    bool SetFolderParentPostId(Folder& folder, const std::string& post_id);
    
    void RenameFolder(const std::string& old_folderpath, 
                      const std::string& new_folderpath,
                      std::deque<FileInfo>& file_list,
                      std::deque<Folder>& folder_list);

    void DeleteFolder(const std::string& folderpath);

    void DeleteFolder(const std::string& folderpath, 
                      std::deque<FileInfo>& file_out,
                      std::deque<Folder>& folder_out);

    void MarkFolderDeleted(FolderPost& fp);

    bool GetFolderById(const std::string& folder_id, Folder& out);
    bool GetFolder(const std::string& folderpath, Folder& out);
    bool IsFolderInCache(const std::string& folder_name, const std::string& parent_post_id);
    bool IsFolderInCacheWithId(const std::string& post_id);
    bool SetFolderDeleted(const std::string& folderpath, bool del);


    int CreateFolderPost(Folder& folder, 
                         const std::string& posts_feed,
                         const AccessToken& at,
                         std::string& id_out);

    bool UpdateFolderPost(Folder& folder, 
                          const std::string& post_id,
                          const std::string& post_path,
                          const AccessToken& at);
 
    int RetrieveFolderPost(const std::string& post_id, 
                           const std::string& post_path,
                           const AccessToken& at,
                           FolderPost& out);
 
    int PostFolderPost(const std::string& post_id, 
                       const std::string& post_path,
                       const AccessToken& at,
                       FolderPost& fp);
 
private:
    FileManager* file_manager_;
};

}//namespace
#endif

