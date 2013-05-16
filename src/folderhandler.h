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

public:
    FolderHandler(FileManager* fm);
    ~FolderHandler();

    bool ValidateFolder(FolderPost& fp);
    bool CreateFolder(const std::string& folderpath, std::deque<Folder>& out);
    void SetFolderPostId(Folder& folder, const std::string& post_id);
    void SetFolderParentPostId(Folder& folder, const std::string& post_id);
    
    void RenameFolder(const std::string& old_folderpath, 
                      const std::string& new_folderpath);

    void DeleteFolder(const std::string& folderpath);
    void MarkFolderDeleted(FolderPost& fp);

private:
    FileManager* file_manager_;
};

}//namespace
#endif

