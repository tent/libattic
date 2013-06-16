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

FileManager::FileManager() : MutexClass() {}
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

    Lock();
    if(!manifest_.file_table()->RemoveFileInfo(filepath))
        status = ret::A_FAIL_FILE_NOT_IN_MANIFEST;
    Unlock();

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
            Lock();
            // Insert into infotable
            ret = manifest_.file_table()->InsertFileInfo(*pFi);
            Unlock();
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
            Lock();
            manifest_.folder_table()->set_folderpath(folder.folder_post_id(), alias_new);
            Unlock();
            //Update folder contents
            Lock();
            bool ret = manifest_.UpdateAllFileInfoForFolder(folder.folder_post_id());
            Unlock();
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
    FileInfo* fi = GetFileInfo(alias_old);
    if(fi && !alias_new.empty()) {
        std::string filename;
        utils::ExtractFileName(alias_new, filename);

        Lock();
        bool s = manifest_.file_table()->set_filepath(alias_old, alias_new); // update filepath
        if(s) s = manifest_.file_table()->set_filename(alias_new, filename); // update filename
        if(s) {
            Folder folder;
            if(manifest_.folder_table()->QueryForFolder(folderpath, folder)) {
                s = manifest_.file_table()->set_folder_post_id(alias_new, folder.folder_post_id());
            }
        }
        Unlock();
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
        Lock();
        ret = manifest_.file_table()->set_file_version(filepath, version);
        Unlock();
    }
    return ret;
}

bool FileManager::SetFileDeleted(const std::string& filepath, const bool del) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        Lock();
        ret = manifest_.file_table()->set_file_deleted(filepath, del);
        Unlock();
    }
    return ret;
}

bool FileManager::MarkFilesInFolderDeleted(const std::string& folder_id) {
    Lock();
    bool ret = manifest_.file_table()->MarkAllFilesDeletedInFolder(folder_id);
    Unlock();
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
        Lock();
        manifest_.file_table()->set_file_post_id(relative, postid);
        Unlock();
    }
    else
        return false;
    return true;
}

// Expecting relative path, "relative to working dir, ie : <working>/path/to/file"
bool FileManager::SetFileFolderPostId(const std::string& filepath, const std::string& postid) {
    bool ret = false;
    if(IsPathRelative(filepath)) {
        Lock();
        ret = manifest_.file_table()->set_folder_post_id(filepath, postid);
        Unlock();
    }
    return ret;
}

bool FileManager::SetNewFilepath(const std::string& old_filepath, const std::string& new_filepath) {
    bool ret = false;
    if(IsPathRelative(old_filepath) && IsPathRelative(new_filepath)) {
        Lock();
        ret = manifest_.file_table()->set_filepath(old_filepath, new_filepath);
        Unlock();
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
        Lock();
        ret = manifest_.file_table()->set_chunk_count(filepath, count);
        Unlock();
    }
    return ret;
}

FileInfo* FileManager::CreateFileInfo() {
    Lock();
    FileInfo* pFi = file_info_factory_.CreateFileInfoObject();
    Unlock();

    return pFi;
}

bool FileManager::GetAliasedFilepath(const std::string& filepath, std::string& out) {
    size_t pos = filepath.find(working_directory_);
    if(pos != std::string::npos) {
        out = std::string(cnst::g_szWorkingPlaceHolder);
        std::string right = filepath.substr(pos+working_directory_.size());
        if(right.size())
            out += "/" + right;
        pos = out.find("//");
        if(pos != std::string::npos) {
            out.erase(pos, 1);
        }
        return true;
    }
    return false;
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
        Lock();
        stat = manifest_.file_table()->IsFileInManifest(relative);
        Unlock();
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
        Lock();
        working = working_directory_;
        Unlock();
        
        fs::MakePathRelative(working, canonical, relative);
        out = std::string(cnst::g_szWorkingPlaceHolder) + "/" + relative;
        retval = true;
    }
    else {
        std::cout<< " could not find working directory within canonical path " << std::endl;
    }
    return retval;
} 




FileInfo* FileManager::GetFileInfoByPostId(const std::string& post_id) {
    FileInfo* fi = NULL;

    if(!post_id.empty()) {
        Lock();
        fi = file_info_factory_.CreateFileInfoObject();
        manifest_.file_table()->QueryForFileByPostId(post_id, *fi);
        Unlock();
    }

    return fi;
}

bool FileManager::GetFileInfoByPostId(const std::string& post_id, FileInfo& out) {
    Lock();
    bool ret = manifest_.file_table()->QueryForFileByPostId(post_id, out);
    Unlock();
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
        Lock();
        ret = manifest_.file_table()->IsFileInManifest(relative);
        if(ret)
            manifest_.file_table()->QueryForFile(relative, out);
        Unlock();
    }
    else {
        std::cout<<"GETFILEINFO FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return ret;
}

FileInfo* FileManager::GetFileInfo(const std::string &filepath) {
    FileInfo* pFi = NULL;
    std::string relative;
    if(!IsPathRelative(filepath)) {
        AttemptToGetRelativePath(filepath, relative);
    } 
    else
        relative = filepath;

    // Attempt to get relative path
    if(IsPathRelative(relative)) {
        Lock();
        pFi = file_info_factory_.CreateFileInfoObject();
        manifest_.file_table()->QueryForFile(relative, *pFi);
        Unlock();

        if(pFi) {
            if(!pFi->IsValid()) {
                pFi = NULL;
            }
        }
        else {
            std::cout<<" NULL FileInfo " << std::endl;
        }
    }
    else {
        std::cout<<"GETFILEINFO FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return pFi;
}


bool FileManager::GetFolderEntryByPostId(const std::string& post_id, Folder& folder) {
    bool ret = false;
    if(!post_id.empty()) {
        Lock();
        ret = manifest_.folder_table()->QueryForFolderByPostId(post_id, folder);
        Unlock();
    }
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

    Lock();
    bool ret = manifest_.folder_table()->QueryForFolder(relative, folder);
    Unlock();
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
    Lock();
    bool ret = manifest_.folder_table()->IsFolderInManifest(relative);
    Unlock();
    return ret;
}

bool FileManager::DoesFolderExistById(const std::string& post_id) {
    bool ret = false;
    if(!post_id.empty()) {
        Lock();
        ret = manifest_.folder_table()->IsFolderInManifestWithID(post_id);
        Unlock();
    }
    return ret;
}

bool FileManager::GetFolderPostId(const std::string& folderpath, std::string& id_out) { 
    Folder folder;
    bool ret = GetFolderEntry(folderpath, folder);
    if(ret)
        id_out = folder.folder_post_id();
    return ret;
}

bool FileManager::CreateFolderEntry(const std::string& folderpath, 
                                    const std::string& folder_post_id,
                                    const std::string& parent_post_id,
                                    Folder& out) {
    // Create Folder Entry
    std::string relative;
    if(!IsPathRelative(folderpath)) {
        GetRelativePath(folderpath, relative);
    }
    else {
        relative = folderpath;
    }

    if(relative.empty())
        relative = cnst::g_szWorkingPlaceHolder;

    std::string p_post_id = parent_post_id;
    if(relative == cnst::g_szWorkingPlaceHolder && parent_post_id.empty())
        p_post_id = cnst::g_szWorkingPlaceHolder;

    // Normalize Folderpath
    utils::CheckUrlAndRemoveTrailingSlash(relative);
    bool ret = false;
    ret = GetFolderEntry(relative, out);
    if(!ret) {
        Lock();
        ret = manifest_.folder_table()->InsertFolderInfo(relative, folder_post_id, p_post_id, false);
        if(ret)
            ret = manifest_.folder_table()->QueryForFolder(relative, out);
        Unlock();
    }
    return ret;
}

// TODO :: pass in filepath and id as string, no need for filemanager to know about folderpost
bool FileManager::UpdateFolderEntry(const std::string& folderpath, const std::string& post_id) {
    bool ret = false;
    Lock();
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
    Unlock();
    return ret;
}



bool FileManager::UpdateFolderContents(Folder& folder) {
    bool ret = false;
    Lock();
    ret = manifest_.UpdateAllFileInfoForFolder(folder.folder_post_id());
    Unlock();

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
    Lock();
    manifest_.folder_table()->GetFolderId(relative, id);
    Unlock();
    if(id.empty()) {
        Lock();
        if(manifest_.folder_table()->IsFolderInManifest(relative))
            ret = manifest_.folder_table()->set_folder_post_id(relative, post_id);
        Unlock();
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
    Lock();
    if(manifest_.folder_table()->IsFolderInManifest(relative))
        ret = manifest_.folder_table()->set_folder_parent_post_id(relative, post_id);
    Unlock();
    return ret;
}

bool FileManager::SetFolderDeleted(const std::string& folderpath, bool del) {
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
    Lock();
    if(manifest_.folder_table()->IsFolderInManifest(relative))
        ret = manifest_.folder_table()->set_folder_deleted(relative, del);
    Unlock();
    return ret;
}

int FileManager::GetAllFileInfo(std::deque<FileInfo>& out) {
    Lock();
    int status = manifest_.file_table()->QueryAllFiles(out);
    Unlock();
    return status;
}

int FileManager::GetAllFileInfoForFolder(const std::string& folderid, std::deque<FileInfo>& out) {
    Lock();
    int status = manifest_.file_table()->QueryAllFilesForFolder(folderid, out);
    Unlock();
    return status;
}

int FileManager::GetAllFoldersForFolder(const std::string& folderid, std::deque<Folder>& out) {
    Lock();
    int status = manifest_.folder_table()->QueryAllFoldersForFolder(folderid, out);
    Unlock();
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

bool FileManager::AddWorkingDirectory(const std::string& directory_path) {
    bool ret = false;
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


}//namespace
