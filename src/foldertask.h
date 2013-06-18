#ifndef FOLDERTASK_H_
#define FOLDERTASK_H_
#pragma once

#include <deque>
#include <string>
#include "tenttask.h"
#include "filepost.h"
#include "folderpost.h"

namespace attic {

class FolderTask : public TentTask{
    int CreateFolderHierarchyList(const std::string& folderpath, std::deque<std::string>& out);
    bool CheckFolderDeleted(const std::string& folderpath);

    int CreateFolderPost(Folder& folder, std::string& id_out);
    int CreateFolder(const std::string& path);

    int DeleteFolder();
    int MarkFolderPostDeleted(Folder& folder);
    int MarkFilePostDeleted(FileInfo& fi);

    int RenameFolder();

    bool UpdateFilePost(FileInfo& fi, const std::string post_id);
    bool UpdateFolderPost(Folder& folder, const std::string post_id);

    int RetrieveFilePost(const std::string& post_id, FilePost& out);
    int PostFilePost(const std::string& post_id, FilePost& fp);
    int RetrieveFolderPost(const std::string& post_id, FolderPost& out);
    int PostFolderPost(const std::string& post_id, FolderPost& fp);

    void SeparatePath(const std::string& full_path, std::deque<std::string>& names);
public:
    FolderTask(FileManager* pFm,
               CredentialsManager* pCm,
               const AccessToken& at,
               const Entity& entity,
               const TaskContext& context);

    ~FolderTask();

    virtual void OnStart() {} 
    virtual void OnPaused() {}
    virtual void OnFinished() {}

    void RunTask();
};

}//namespace
#endif

