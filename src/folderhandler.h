#ifndef FOLDERHANDLER_H_
#define FOLDERHANDLER_H_
#pragma once

#include <string>
#include <deque>
#include "folder.h"
#include "folderpost.h"
#include "filemanager.h"

namespace attic {

class FolderHandler {
    int RetrieveSubFolders(Folder& folder, std::deque<Folder>& out);
    int RetrieveFilesInFolder(Folder& folder, std::deque<FileInfo>& out);
    int RetrieveAllFilesAndFoldersInFolder(Folder& folder, 
                                           std::deque<FileInfo>& file_out,
                                           std::deque<Folder>& folder_out);
public:
    FolderHandler(FileManager* fm);
    ~FolderHandler();

    bool ValidateFolder(FolderPost& fp);
    bool CreateFolder(const std::string& folderpath, std::deque<Folder>& out);
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

private:
    FileManager* file_manager_;
};

}//namespace
#endif

