#include "filemanager.h"

#include <string>
#include <iostream>

#include "filesystem.h"
#include "utils.h"
#include "chunkinfo.h"
#include "folder.h"
#include "constants.h"
#include "logutils.h"

namespace attic {

FileManager::FileManager() {}
FileManager::~FileManager() {}

int FileManager::Initialize(const std::string &manifestDirectory, 
                            const std::string &workingDirectory,
                            const std::string& tempDirectory) {
    // Set manifest path
    manifest_directory_ = manifestDirectory;
    manifest_.SetDirectory(manifest_directory_);

    // Set working directory
    working_directory_ = workingDirectory;
    temp_directory_ = tempDirectory;

    return manifest_.Initialize();
}

int FileManager::Shutdown() {
    return manifest_.Shutdown();
}

int FileManager::RemoveFile(const std::string &filepath) {
    int status = ret::A_OK;

    manifest_mtx_.Lock();
    if(!manifest_.file_table()->RemoveFileInfo(filepath))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    manifest_mtx_.Unlock();

    return status;
}

void FileManager::GetRelativePath(const std::string& filepath, std::string& relative_out) {
    if(IsPathRelative(filepath)) {
        relative_out = filepath;
        return;
    }

    std::string canonical, relative;
    if(!filepath.empty()) {
        fs::GetCanonicalPath(filepath, canonical);
        fs::MakePathRelative(working_directory_, canonical, relative);
    }
    relative_out = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
}

void FileManager::ExtractRelativePaths(const FileInfo* pFi, 
                                       std::string& relative_out, 
                                       std::string& parent_relative_out) {
    std::string filepath = pFi->filepath();
    std::string relative, canonical;

    // If filepath is already relative
    if(IsPathRelative(filepath)) {
        // set canonical
        int pos = filepath.find(cnst::g_szWorkingPlaceHolder);
        canonical = working_directory_ + filepath.substr((pos+strlen(cnst::g_szWorkingPlaceHolder)));
        relative = filepath;
    }
    else {
        fs::GetCanonicalPath(filepath, canonical);
        fs::MakePathRelative(working_directory_, canonical, relative);
        // TODO :: fix for windows
        relative = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
        //if(parent_relative.empty())
        if(canonical.empty()) {
            std::cout<<" FILEMANAGER - CANONICAL IS EMPTY - " << filepath << std::endl;
        }
    }
    // Extract parent relative :
    std::string filename = pFi->filename();
    int pos = relative.find(filename);
    std::string parent_relative;
    if(pos != std::string::npos)
        parent_relative = relative.substr(0, pos-1); // minus 1 to include /
    relative_out = relative;
    parent_relative_out = parent_relative;
}

bool FileManager::InsertToManifest (FileInfo* pFi) { 
    bool ret = false;
    if(pFi) {
        if(!pFi->filepath().empty()) {
            manifest_mtx_.Lock();
            // Insert into infotable
            ret = manifest_.file_table()->InsertFileInfo(*pFi);
            manifest_mtx_.Unlock();
        }
    }
    return ret;
}

int FileManager::RenameFolder(const std::string& old_folderpath,
                              const std::string& new_folderpath) {
    int status = ret::A_OK;
    std::string alias_old, alias_new;
    if(!IsPathRelative(old_folderpath))
        GetAliasedFilepath(old_folderpath, alias_old);
    else
        alias_old = old_folderpath;

    if(!IsPathRelative(new_folderpath))
        GetAliasedFilepath(new_folderpath, alias_new);
    else
        alias_new = new_folderpath;
    Folder folder;
    if(GetFolderEntry(alias_old, folder)){
        if(!alias_new.empty()) {
            //Update folder path
            manifest_mtx_.Lock();
            manifest_.folder_table()->set_foldername(folder.folder_post_id(), alias_new);
            manifest_mtx_.Unlock();
            //Update folder contents
            manifest_mtx_.Lock();
            bool ret = manifest_.UpdateAllFileInfoForFolder(folder.folder_post_id());
            manifest_mtx_.Unlock();
        }
        else {
            status = ret::A_FAIL_INVALID_FOLDERPATH;
        }
    }
    else {
        status = ret::A_FAIL_FOLDER_NOT_IN_MANIFEST;
    }

    return status;
}

// Takes in two absolute (canonical) filepaths, and swaps them
int FileManager::RenameFile(const std::string& old_filepath, 
                            const std::string& new_filepath) {
    int status = ret::A_OK;
    std::string alias_old, alias_new;
    if(!IsPathRelative(old_filepath))
        GetAliasedFilepath(old_filepath, alias_old);
    else
        alias_old = old_filepath;

    if(!IsPathRelative(new_filepath))
        GetAliasedFilepath(new_filepath, alias_new);
    else
        alias_new = new_filepath;

    // retrieve new folder post
    std::string folderpath;
    utils::ExtractFolderpath(alias_new, folderpath);
    if(manifest_.file_table()->IsFileInManifest(alias_old) && !alias_new.empty()) {
        std::string filename;
        utils::ExtractFileName(alias_new, filename);
        manifest_mtx_.Lock();
        bool s = manifest_.file_table()->set_filepath(alias_old, alias_new); // update filepath
        if(s) s = manifest_.file_table()->set_filename(alias_new, filename); // update filename
        if(s) {
            Folder folder;
            if(manifest_.folder_table()->QueryForFolder(folderpath, folder)) {
                s = manifest_.file_table()->set_folder_post_id(alias_new, folder.folder_post_id());
            }
        }
        manifest_mtx_.Unlock();
        if(!s) {
            status = ret::A_FAIL_TO_QUERY_MANIFEST;
        }
    }
    else {
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    }
    return status;
}

bool FileManager::SetFileVersion(const std::string& filepath, const std::string& version) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->set_file_version(filepath, version);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::SetFileDeleted(const std::string& filepath, const bool del) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->set_file_deleted(filepath, del);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::MarkFilesInFolderDeleted(const std::string& folder_id) {
    manifest_mtx_.Lock();
    bool ret = manifest_.file_table()->MarkAllFilesDeletedInFolder(folder_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::MarkFilesInFolderDeleted(const Folder& folder) {
    return MarkFilesInFolderDeleted(folder.folder_post_id());
}

bool FileManager::SetFilePostId(const std::string &filepath, const std::string& postid) {
    std::string relative;
    if(!IsPathRelative(filepath)) {
        AttemptToGetRelativePath(filepath, relative);
    } 
    else
        relative = filepath;

    if(!relative.empty()) {
        manifest_mtx_.Lock();
        manifest_.file_table()->set_file_post_id(relative, postid);
        manifest_mtx_.Unlock();
    }
    else
        return false;
    return true;
}

// Expecting relative path, "relative to working dir, ie : <working>/path/to/file"
bool FileManager::SetFileFolderPostId(const std::string& filepath, const std::string& postid) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->set_folder_post_id(filepath, postid);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::SetNewFilepath(const std::string& old_filepath, const std::string& new_filepath) {
    bool ret = false;
    if(IsPathRelative(old_filepath) && IsPathRelative(new_filepath)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->set_filepath(old_filepath, new_filepath);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::SetFileChunks(const std::string& filepath, FileInfo::ChunkMap& map) {
    FileInfo fi;
    if(GetFileInfo(filepath, fi)) {
        fi.set_chunks(map);
        return InsertToManifest(&fi);
    }
    return false;
}

bool FileManager::SetFileChunkCount(const std::string& filepath, const std::string& count) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->set_chunk_count(filepath, count);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::GetAliasedFilepath(const std::string& filepath, std::string& out) {
    bool ret = false;
    std::string directory, dir_post_id;
    if(FindAssociatedWorkingDirectory(filepath, directory, dir_post_id)) {
        // Get alias
        std::string alias;
        manifest_mtx_.Lock();
        ret = manifest_.config_table()->RetrieveConfigValue(dir_post_id, alias);
        manifest_mtx_.Unlock();
        size_t pos = filepath.find(directory);
        out += alias + "/" + filepath.substr(pos+directory.size());
    }
    return ret;
}

bool FileManager::GetCanonicalFilepath(const std::string& relativepath, std::string& out) {
    std::string relative, canonical;
    if(IsPathRelative(relativepath)) {
        // Extract second half of path
        std::string right_fp;
        size_t left = relativepath.find("/");
        if(left != std::string::npos) {
            right_fp = relativepath.substr(left);
        }
        // Replace working
        size_t pos = relativepath.find(cnst::g_szWorkingPlaceHolder);
        if(pos != std::string::npos) {
            if(!right_fp.empty()) {
            out = working_directory_ + "/" + relativepath.substr(pos + strlen(cnst::g_szWorkingPlaceHolder) + 1);
            }
            else{ 
                out = working_directory_;
            }
            return true;
        }
        else {
            std::cout << " MALFORMED RELATIVE PATH " << relativepath << std::endl;
        }
    }
    else {
        std::cout<< " PATH NOT RELATIVE " << std::endl;
    }

    return false;
}

bool FileManager::IsPathRelative(const std::string& filepath) {
    if(filepath.find(cnst::g_szWorkingPlaceHolder) != std::string::npos)
        return true;
    return false;
}

bool FileManager::DoesFileExist(const std::string& filepath) {
    bool stat = false;
    std::string relative;
    if(AttemptToGetRelativePath(filepath, relative)) {
        manifest_mtx_.Lock();
        stat = manifest_.file_table()->IsFileInManifest(relative);
        manifest_mtx_.Unlock();
    }
    else {
        std::cout<<"DoesFileExist FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return stat;
}

bool FileManager::AttemptToGetRelativePath(const std::string& filepath, std::string& out) {
    bool retval = false;

    if(filepath.find(working_directory_) != std::string::npos) {
        std::string relative;
        fs::MakePathRelative(working_directory_, filepath, relative);
        out = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
        retval = true;
        return retval;
    }

    std::string rel;
    fs::MakePathRelative(working_directory_, filepath, rel);
    std::string canonical;
    int pos = 0;
    int last = 0;
    while(pos != std::string::npos) {
        last = pos;
        pos = filepath.find("/", pos + 1);
    }

    if(last) {
        fs::GetCanonicalPath(filepath.substr(0, pos-1), canonical);
        //canonical += filepath.substr(pos+1);
    }
    else {
        fs::GetCanonicalPath(filepath, canonical);
    }

    if(canonical.find(working_directory_) != std::string::npos) {
        std::string relative, working;
        manifest_mtx_.Lock();
        working = working_directory_;
        manifest_mtx_.Unlock();
        
        fs::MakePathRelative(working, canonical, relative);
        out = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
        retval = true;
    }
    else {
        std::cout<< " could not find working directory within canonical path " << std::endl;
    }
    return retval;
} 

bool FileManager::GetFileInfoByPostId(const std::string& post_id, FileInfo& out) {
    manifest_mtx_.Lock();
    bool ret = manifest_.file_table()->QueryForFileByPostId(post_id, out);
    manifest_mtx_.Unlock();
    return ret;
}


bool FileManager::GetFileInfo(const std::string& filepath, FileInfo& out) {
    std::string relative;
    if(!IsPathRelative(filepath)) {
        AttemptToGetRelativePath(filepath, relative);
    } 
    else
        relative = filepath;

    bool ret = false;
    // Attempt to get relative path
    if(IsPathRelative(relative)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->IsFileInManifest(relative);
        if(ret)
            manifest_.file_table()->QueryForFile(relative, out);
        manifest_mtx_.Unlock();
    }
    else {
        std::cout<<"GETFILEINFO FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return ret;
}

bool FileManager::GetFolderEntryByPostId(const std::string& post_id, Folder& folder) {
    bool ret = false;
    if(!post_id.empty()) {
        manifest_mtx_.Lock();
        ret = manifest_.folder_table()->QueryForFolderByPostId(post_id, folder);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::GetFolderEntry(const std::string& foldername, 
                                 const std::string& parent_post_id,
                                 Folder& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.folder_table()->QueryForFolder(foldername, parent_post_id, out);
    std::cout<<" get folder entry : " << foldername << " id : " << parent_post_id << " found? : " << ret << std::endl;
    manifest_mtx_.Unlock();
    return ret;
}

// Pass in absoulte folder path
bool FileManager::GetFolderEntry(const std::string& folderpath, Folder& folder) {
    // Check if folder exists
    //  - if it does, fill out class
    //  - else return false
    std::string relative;
    if(!IsPathRelative(folderpath)) {
        // Make Relative
        //GetRelativePath(folderpath, relative);
        GetAliasedFilepath(folderpath, relative);
    }
    else {
        relative = folderpath;
    }
    if(relative.empty())
        relative = cnst::g_szWorkingPlaceHolder;
    // Normalize Folderpath
    utils::CheckUrlAndRemoveTrailingSlash(relative);
    std::cout<<" folder entry relative path : " << relative << std::endl;

    manifest_mtx_.Lock();
    bool ret = manifest_.folder_table()->QueryForFolder(relative, folder);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::DoesFolderExist(const std::string& folderpath) {
    std::string relative;
    if(!IsPathRelative(folderpath))
        GetAliasedFilepath(folderpath, relative);
    else
        relative = folderpath;

    if(relative.empty())
        relative = cnst::g_szWorkingPlaceHolder;
    manifest_mtx_.Lock();
    bool ret = manifest_.folder_table()->IsFolderInManifest(relative);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::DoesFolderExistById(const std::string& post_id) {
    bool ret = false;
    if(!post_id.empty()) {
        manifest_mtx_.Lock();
        ret = manifest_.folder_table()->IsFolderInManifest(post_id);
        manifest_mtx_.Unlock();
    }
    return ret;
}

bool FileManager::GetFolderPostId(const std::string& folderpath, std::string& id_out) { 
    bool ret = false;
    // Determine root post;
    std::string directory, dir_post_id;
    if(FindAssociatedWorkingDirectory(folderpath, directory, dir_post_id)) {
        std::cout<<" directory : " << directory << std::endl;
        std::cout<<" dir_post_id : " << dir_post_id << std::endl;
        if(folderpath == directory) {
            id_out = dir_post_id;
            ret = true;
        }
        else {
            // Get last foldername
            // query for all children who have this as a perent
            // find the corresponding foldername
            // keep going till the last post is found
        }
    }
    return ret;
}

bool FileManager::CreateFolderEntry(const std::string& foldername,
                                    const std::string& folder_post_id,
                                    const std::string& parent_post_id,
                                    Folder& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(!manifest_.folder_table()->IsFolderInManifest(folder_post_id)) {
        ret = manifest_.folder_table()->InsertFolderInfo(foldername, 
                                                         folder_post_id, 
                                                         parent_post_id, 
                                                         false);
        if(ret)
            ret = manifest_.folder_table()->QueryForFolderByPostId(folder_post_id, out);
    }
    manifest_mtx_.Unlock();
    return ret;
}

// TODO :: pass in filepath and id as string, no need for filemanager to know about folderpost
bool FileManager::UpdateFolderEntry(const std::string& folderpath, const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(folderpath)) {
        std::string id;
        manifest_.folder_table()->GetFolderPostID(folderpath, id);
        if(id.empty()) {
            // update id
            ret = manifest_.folder_table()->set_folder_post_id(folderpath, post_id);
        }
        else if(id != post_id) {
            // throw error
            std::string error = " LOCAL CACHE FOLDER POST ID MISMATCH \n";
            error += "\tpost id : ";
            error += post_id;
            error += "\n";
            error += "\t conficted id : ";
            error += id;
            error += "\n";
            error += "folderpath : ";
            error += folderpath;
            log::LogString("MASDlf", error);
        }
        else {
            // Already ok
            ret = true;
        }
    }
    manifest_mtx_.Unlock();
    return ret;
}



bool FileManager::UpdateFolderContents(Folder& folder) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.UpdateAllFileInfoForFolder(folder.folder_post_id());
    manifest_mtx_.Unlock();

    return ret;
}

bool FileManager::SetFolderPostId(const std::string& folderpath, const std::string& post_id) {
    std::string relative;
    if(!IsPathRelative(folderpath)) {
        GetRelativePath(folderpath, relative);
    }
    else {
        relative = folderpath;
    }

    if(relative.empty())
        relative = cnst::g_szWorkingPlaceHolder;
    bool ret = false;
    std::string id;
    manifest_mtx_.Lock();
    manifest_.folder_table()->GetFolderId(relative, id);
    manifest_mtx_.Unlock();
    if(id.empty()) {
        manifest_mtx_.Lock();
        if(manifest_.folder_table()->IsFolderInManifest(relative))
            ret = manifest_.folder_table()->set_folder_post_id(relative, post_id);
        manifest_mtx_.Unlock();
    }
    else {
        std::ostringstream err;
        err << "Set folder post id collision " << std::endl;
        err << " id already set : " << id << std::endl;
        err << " trying to insert id : " <<  post_id << std::endl;
        log::LogString("dakm102", err.str());
    }
    return ret;
}

bool FileManager::SetFolderParentPostId(const std::string& folderpath, const std::string& post_id) {
    std::string relative;
    if(!IsPathRelative(folderpath)) {
        GetRelativePath(folderpath, relative);
    }
    else {
        relative = folderpath;
    }
    if(relative.empty())
        relative = cnst::g_szWorkingPlaceHolder;
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(relative))
        ret = manifest_.folder_table()->set_folder_parent_post_id(relative, post_id);
    manifest_mtx_.Unlock();
    return ret;
}

// Kind of a worrysome method, revist at some point
bool FileManager::IsFolderDeleted(const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id))
        ret = manifest_.folder_table()->IsFolderDeleted(post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFolderDeleted(const std::string& post_id, bool del) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id))
        ret = manifest_.folder_table()->set_folder_deleted(post_id, del);
    manifest_mtx_.Unlock();
    return ret;
}

int FileManager::GetAllFileInfo(std::deque<FileInfo>& out) {
    manifest_mtx_.Lock();
    int status = manifest_.file_table()->QueryAllFiles(out);
    manifest_mtx_.Unlock();
    return status;
}

int FileManager::GetAllFileInfoForFolder(const std::string& folderid, std::deque<FileInfo>& out) {
    manifest_mtx_.Lock();
    int status = manifest_.file_table()->QueryAllFilesForFolder(folderid, out);
    manifest_mtx_.Unlock();
    return status;
}

int FileManager::GetAllFoldersForFolder(const std::string& folderid, std::deque<Folder>& out) {
    manifest_mtx_.Lock();
    int status = manifest_.folder_table()->QueryAllFoldersForFolder(folderid, out);
    manifest_mtx_.Unlock();
    return status;
}

bool FileManager::LockFile(const std::string& filepath) {
    return file_queue_.LockFile(filepath);
}

bool FileManager::UnlockFile(const std::string& filepath) {
    return file_queue_.UnlockFile(filepath);
}

bool FileManager::IsFileLocked(const std::string& filepath) {
    return file_queue_.IsFileLocked(filepath);
}

// Data layout for working directory entries are as follows
//  type - root_dir
//  key - alias
//  value - post id
//  state - local (absolute) filepath associated with this machine
bool FileManager::AddWorkingDirectory(const std::string& directory_alias, 
                                      const std::string& directory_path, 
                                      const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    // Local directory (should be an absolute path, linked to a corresponding root folder post)
    if(!manifest_.config_table()->IsStateInManifest(directory_path)) {
        ret = manifest_.config_table()->InsertConfigValue(config::dir_type, 
                                                          post_id, 
                                                          directory_alias,
                                                          directory_path);
    }
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::IsDirectoryLinked(const std::string& directory_path) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->IsStateInManifest(directory_path);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::UnlinkWorkingDirectory(const std::string& directory_path) {
    bool ret = false;
    return ret;
}

bool FileManager::RemoveWorkingDirectory(const std::string& directory_path) {
    bool ret = false;
    return ret;
}

bool FileManager::RetrieveAllConfigEntries(std::deque<ConfigEntry>& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    manifest_.config_table()->RetrieveAllEntries(out);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::HasConfigValue(const std::string& key) {
    bool ret = false;
    return ret;
}

bool FileManager::GetConfigValue(const std::string& key) {
    bool ret = false;
    return ret;
}

bool FileManager::PushConfigValue(const std::string& type, 
                                  const std::string& key, 
                                  const std::string& value) {
    bool ret = false;
    return ret;
}

bool FileManager::LoadWorkingDirectories() {
    bool ret = false;
    std::deque<ConfigEntry> entries;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigType(config::dir_type, entries);
    manifest_mtx_.Unlock();

    std::cout<< " Loading Working directories " << std::endl;
    std::cout<< " Entry count : " << entries.size() << std::endl;

    working_mtx_.Lock();
    working_directories_.clear();
    std::deque<ConfigEntry>::iterator itr = entries.begin();
    for(;itr!=entries.end(); itr++) {
        std::cout<<" loading : " << (*itr).value << std::endl;
        std::cout<<" path : " << (*itr).state << std::endl;
        working_directories_[(*itr).value] = (*itr).state;
    }
    working_mtx_.Unlock();

    return ret;
}

bool FileManager::FindAssociatedWorkingDirectory(const std::string& filepath, 
                                                 std::string& dir_out,
                                                 std::string& post_id) {
    bool ret = false;
    working_mtx_.Lock();
    std::map<std::string, std::string>::iterator itr = working_directories_.begin(); 
    for(;itr!=working_directories_.end(); itr++) {
        if(filepath.find(itr->second) != std::string::npos) {
            dir_out = itr->second;
            ret = true;
            break;
        }
    }
    working_mtx_.Unlock();
    //Find post id, the config table key has the id, the state is the filepath
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigKeyByState(dir_out, post_id);
    manifest_mtx_.Unlock();
    return ret;
}
bool FileManager::ConstructFolderpath(const std::string post_id, const std::string& path_out) {
    bool ret = false;


    return ret;
}




}//namespace
