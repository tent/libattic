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

void FileManager::ExtractRelativePaths(const FileInfo* pFi, 
                                       std::string& relative_out, 
                                       std::string& parent_relative_out) {
    std::string filepath = pFi->filepath();
    std::string relative, canonical;

    // If filepath is already relative
    if(IsPathAliased(filepath)) {
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

bool FileManager::InsertToManifest (FileInfo* fi) { 
    bool ret = false;
    if(fi) {
        if(!fi->filepath().empty()) {
            manifest_mtx_.Lock();
            // Insert into infotable
            std::string chunk_data;
            fi->GetSerializedChunkData(chunk_data);
            //ret = manifest_.file_table()->InsertFileInfo(*fi);
            ret = manifest_.file_table()->InsertFileInfo(fi->filename(),
                                                         fi->filepath(),
                                                         fi->chunk_count(),
                                                         chunk_data,
                                                         fi->file_size(),
                                                         fi->post_id(),
                                                         fi->post_version(),
                                                         fi->encrypted_key(),
                                                         fi->file_credentials_iv(),
                                                         fi->deleted(),
                                                         fi->folder_post_id(),
                                                         fi->plaintext_hash());
            manifest_mtx_.Unlock();
        }
    }
    return ret;
}


bool FileManager::SetFileVersion(const std::string& post_id, const std::string& version) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.file_table()->set_file_version_for_id(post_id, version);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFileDeleted(const std::string& post_id, const bool del) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.file_table()->set_file_deleted_for_id(post_id, del);
    manifest_mtx_.Unlock();
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

// Expecting relative path, "relative to working dir, ie : <working>/path/to/file"
bool FileManager::SetFileFolderPostId(const std::string& post_id, const std::string& folder_post_id){
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.file_table()->set_folder_post_id(post_id, folder_post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFilename(const std::string& post_id, const std::string& filename) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.file_table()->set_filename_for_id(post_id, filename);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFilepath(const std::string& post_id, const std::string& filepath) {
    bool ret = false;
    std::string aliased_path;
    if(!IsPathAliased(filepath))
        if(!GetAliasedPath(filepath, aliased_path)) return false;
    else 
        aliased_path = filepath;

    manifest_mtx_.Lock();
    ret = manifest_.file_table()->set_filepath_for_id(post_id, filepath);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::GetAliasedPath(const std::string& filepath, std::string& out) {
    bool ret = false;
    std::cout<<" Get aliased path for : " << filepath << std::endl;
    if(!IsPathAliased(filepath)) {
        std::string directory, dir_post_id;
        if(FindAssociatedWorkingDirectory(filepath, directory, dir_post_id)) {
            // Get alias
            std::string alias;
            manifest_mtx_.Lock();
            ret = manifest_.config_table()->RetrieveConfigKeyByValue(dir_post_id, alias);
            manifest_mtx_.Unlock();
            if(ret) {
                size_t pos = filepath.find(directory);
                out += alias + "/" + filepath.substr(pos+directory.size());
                utils::ErrorCheckPathDoubleQuotes(out);
            }
        }
    }
    else {
        out = filepath;
        ret = true;
    }
    return ret;
}

bool FileManager::GetCanonicalPath(const std::string& relativepath, std::string& out) {
    bool ret = false;
    std::ostringstream err;
    err << " ************************************************************** " << std::endl;
    err << " relative path : " << relativepath << std::endl;
    if(IsPathAliased(relativepath)) {
        std::string aliased_directory, post_id;
        if(FindAssociatedCanonicalDirectory(relativepath, aliased_directory, post_id)) {
            err <<" aliased directory : " << aliased_directory << std::endl;
            err <<" post_id : " << post_id << std::endl;
            size_t sp = relativepath.find("/");
            if(sp != std::string::npos) {
                // Replace working
                size_t pos = relativepath.find(aliased_directory);
                out = aliased_directory + "/" + relativepath.substr(sp);
                utils::ErrorCheckPathDoubleQuotes(out);
                ret = true;
            }
            else {
                out = aliased_directory;
                ret = true;
            }
        }
        else {
            err << " FindAssociatedCanonicalDirectory fail " << std::endl;
        }
    }
    else {
        err << " PATH NOT RELATIVE " << std::endl;
    }
    err << " ************************************************************** " << std::endl;
    std::cout << err.str() << std::endl;
    return ret;
}

bool FileManager::IsPathAliased(const std::string& filepath) {
    if(filepath.size() && (filepath.find(cnst::g_alias_prefix) != std::string::npos))
        return true;
    return false;
}

bool FileManager::DoesFileExist(const std::string& filepath) {
    bool ret = false;
    std::string relative;
    if(GetAliasedPath(filepath, relative)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->IsFileInManifest(relative);
        manifest_mtx_.Unlock();
    }
    else {
        std::cout<<"DoesFileExist FILEPATH PASSED NOT RELATIVE : "<< filepath << std::endl;
    }

    return ret;
}

bool FileManager::DoesFileExistWithPostId(const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.file_table()->IsFileInManifestWithId(post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::GetFileInfoByPostId(const std::string& post_id, FileInfo& out) {
    manifest_mtx_.Lock();
    bool ret = manifest_.file_table()->QueryForFileByPostId(post_id, out);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::GetFileInfo(const std::string& filepath, FileInfo& out) {
    bool ret = false;
    std::string relative;
    if(GetAliasedPath(filepath, relative)) {
        manifest_mtx_.Lock();
        ret = manifest_.file_table()->IsFileInManifest(relative);
        if(ret) ret = manifest_.file_table()->QueryForFile(relative, out);
        manifest_mtx_.Unlock();
    }
    else {
        std::cout<<" PATH NOT ALIASED : " << filepath << " : " << relative << std::endl;
    }
    return ret;
}

// Retrieve file info via the file name and the folder's id that it resides in
bool FileManager::GetFileInfo(const std::string& filename, 
                              const std::string& folder_post_id,
                              FileInfo& out) {
    manifest_mtx_.Lock();
    bool ret = manifest_.file_table()->QueryForFile(filename, folder_post_id, out);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::GetFolderEntryByPostId(const std::string& post_id, Folder& folder) {
    bool ret = false;
    if(!post_id.empty()) {
        std::cout<<" GET folder Entry By Post Id : " << post_id << std::endl;
        manifest_mtx_.Lock();
        ret = manifest_.folder_table()->QueryForFolderByPostId(post_id, folder);
        manifest_mtx_.Unlock();
    }
    std::cout<<" ret : " << ret << " foldername : " << folder.foldername() << std::endl;
    return ret;
}

bool FileManager::GetFolderEntry(const std::string& foldername, 
                                 const std::string& parent_post_id,
                                 Folder& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.folder_table()->QueryForFolder(foldername, parent_post_id, out);
    std::cout<<" get folder entry : " << foldername << " id : " << parent_post_id << " found? : " << ret << std::endl;
    if(!ret) {
        // Check against alias
        ret = manifest_.folder_table()->QueryForFolderWithAlias(foldername, parent_post_id, out);
    }
    manifest_mtx_.Unlock();
    return ret;
}

// Pass in absoulte folder path
bool FileManager::GetFolderEntry(const std::string& folderpath, Folder& folder) {
    bool ret = false;
    std::string post_id;
    if(GetFolderPostId(folderpath, post_id))
        ret = GetFolderEntryByPostId(post_id, folder);
    return ret;
}

bool FileManager::DoesFolderExist(const std::string& foldername, const std::string& parent_post_id) {
    bool ret = false;
    if(!parent_post_id.empty()) {
        manifest_mtx_.Lock();
        ret = manifest_.folder_table()->IsFolderInManifest(foldername, parent_post_id);
        manifest_mtx_.Unlock();
    }
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

bool FileManager::GetFolderParentId(const std::string& post_id, std::string& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.folder_table()->GetParentPostId(post_id, out);
    manifest_mtx_.Unlock();
    return ret;
}
bool FileManager::GetFolderPostId(const std::string& folderpath, std::string& id_out) { 
    bool ret = false;
    // Determine root post;
    std::string directory, dir_post_id;
    if(!IsPathAliased(folderpath)){
        if(FindAssociatedWorkingDirectory(folderpath, directory, dir_post_id)) {
            std::cout<<" directory : " << directory << std::endl;
            std::cout<<" dir_post_id : " << dir_post_id << std::endl;
            if(folderpath == directory) {
                id_out = dir_post_id;
                ret = true;
            }
        }
    }
    else {
        size_t pos = folderpath.find("/");
        if(pos != std::string::npos) {
            directory = folderpath.substr(0, pos);
            manifest_mtx_.Lock();
            manifest_.config_table()->RetrieveConfigKeyByValue(directory, dir_post_id);
            manifest_mtx_.Unlock();
        }
    }

    if(!ret) {
       size_t pos = folderpath.find(directory);
       if(pos != std::string::npos) {
           std::deque<std::string> folders;
           utils::SeparatePath(folderpath.substr(pos+directory.size()), folders);
           // query for all children who have this as a perent
           std::string parent_id =  dir_post_id;
           std::deque<std::string>::iterator itr = folders.begin();
           for(; itr!=folders.end(); itr++) {
               // find the corresponding foldername
               Folder folder;
               if(!GetFolderEntry((*itr), parent_id, folder)) {
                   return false;
               }
               parent_id = folder.folder_post_id();
           }
           id_out = parent_id;
           ret = true;
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
                                                         "", // Leave empty, for no alias
                                                         folder_post_id, 
                                                         parent_post_id, 
                                                         false);
        if(ret) ret = manifest_.folder_table()->QueryForFolderByPostId(folder_post_id, out);
    }
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::ClearFolderAlias(const std::string& folder_post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.folder_table()->set_folder_alias(folder_post_id, "");
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::CreateFolderEntry(const std::string& foldername, 
                                    const std::string& alias,
                                    const std::string& folder_post_id,
                                    const std::string& parent_post_id,
                                    Folder& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(!manifest_.folder_table()->IsFolderInManifest(folder_post_id)) {
        ret = manifest_.folder_table()->InsertFolderInfo(foldername, 
                                                         alias, // Leave empty, for no alias
                                                         folder_post_id, 
                                                         parent_post_id, 
                                                         false);
        if(ret) ret = manifest_.folder_table()->QueryForFolderByPostId(folder_post_id, out);
    }
    manifest_mtx_.Unlock();
    return ret;
}
bool FileManager::RemoveFolderEntry(const std::string& foldername, 
                                    const std::string& parent_post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.folder_table()->RemoveFolderData(foldername, parent_post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::UpdateFolderEntry(const std::string& foldername, const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id)) {
        // Update folder name
        std::string cache_foldername;
        if(manifest_.folder_table()->GetFoldername(post_id, cache_foldername)) {
            if(cache_foldername != foldername) {
                // Update foldername
                ret = manifest_.folder_table()->set_foldername(post_id, foldername);
            }
        }
    }
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFolderPostId(const std::string& post_id, const std::string& new_post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id))
        ret = manifest_.folder_table()->set_folder_post_id(post_id, new_post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFoldername(const std::string& post_id, const std::string& foldername) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id))
        ret = manifest_.folder_table()->set_foldername(post_id, foldername);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::SetFolderParentPostId(const std::string& post_id, 
                                        const std::string& parent_post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.folder_table()->IsFolderInManifest(post_id))
        ret = manifest_.folder_table()->set_folder_parent_post_id(post_id, parent_post_id);
    manifest_mtx_.Unlock();
    return ret;
}

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
    std::cout<<" AddWorkingDirectory " << std::endl;
    std::cout<<"\t post id (value) : " << post_id << std::endl;
    std::cout<<"\t directory alias (key): " << directory_alias << std::endl;
    std::cout<<"\t directory path (state) : " << directory_path << std::endl;
    // Local directory (should be an absolute path, linked to a corresponding root folder post)
    if(!manifest_.config_table()->IsStateInManifest(directory_path)) {
        ret = manifest_.config_table()->InsertConfigValue(cnst::g_config_dir_type, 
                                                          directory_alias, // key
                                                          post_id,  // value
                                                          directory_path); // state
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

bool FileManager::LinkWorkingDirectory(const std::string& config_key,
                                       const std::string& directory_path) {
    bool ret = false;
    manifest_mtx_.Lock();
    if(manifest_.config_table()->IsKeyInManifest(config_key)) 
        ret = manifest_.config_table()->set_state_for_key(config_key, directory_path);
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
    ret = manifest_.config_table()->RetrieveAllEntries(out);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::RetrieveAllConfigEntriesOfType(const std::string& type, 
                                                 std::deque<ConfigEntry>& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigType(type, out);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::HasConfigValue(const std::string& value) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->IsValueInManifest(value);
    manifest_mtx_.Unlock();
    return ret;
}
bool FileManager::HasConfigKey(const std::string& key) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->IsKeyInManifest(key);
    manifest_mtx_.Unlock();

    return ret;
}

bool FileManager::GetConfigEntry(const std::string& key, ConfigEntry& out) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigEntry(key, out);
    manifest_mtx_.Unlock();

    return ret;
}

bool FileManager::PushConfigValue(const std::string& type, 
                                  const std::string& key, 
                                  const std::string& value) {
    bool ret = false;
    std::cout<<" PushConfigValue " << std::endl;
    std::cout<<"\t type : " << type << std::endl;
    std::cout<<"\t key : " << key << std::endl;
    std::cout<<"\t value : " << value << std::endl;
    manifest_mtx_.Lock();
    if(!manifest_.config_table()->IsKeyInManifest(key))
        ret = manifest_.config_table()->InsertConfigValue(type, key, value, "");
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::LoadWorkingDirectories() {
    bool ret = false;
    std::deque<ConfigEntry> entries;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigType(cnst::g_config_dir_type, entries);
    manifest_mtx_.Unlock();

    std::cout<< " Loading Working directories " << std::endl;
    std::cout<< " Entry count : " << entries.size() << std::endl;

    working_mtx_.Lock();
    working_directories_.clear();
    std::deque<ConfigEntry>::iterator itr = entries.begin();
    for(;itr!=entries.end(); itr++) {
        std::cout<<" loading : " << (*itr).value << std::endl;
        std::cout<<" path : " << (*itr).state << std::endl;
        std::cout<<" alias : " << (*itr).config_key << std::endl;
        working_directories_[(*itr).config_key] = (*itr).state;
    }
    working_mtx_.Unlock();

    return ret;
}

// Retruns the fully mapped path of a working directory
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
    ret = manifest_.config_table()->RetrieveConfigValueByState(dir_out, post_id);
    manifest_mtx_.Unlock();
    return ret;
}

// pass in an aliased path, will return the full path of what its mapped to
bool FileManager::FindAssociatedCanonicalDirectory(const std::string& aliased_path,
                                                   std::string& dir_out, 
                                                   std::string& post_id) {
    bool ret = false;
    working_mtx_.Lock();
    std::string alias;
    std::map<std::string, std::string>::iterator itr = working_directories_.begin(); 
    for(;itr!=working_directories_.end(); itr++) {
        std::cout<<" checking : " << itr->first << std::endl;
        if(aliased_path.find(itr->first) != std::string::npos) {
            dir_out = itr->second;
            alias = itr->first;
            std::cout<<" found : " << dir_out << std::endl;
            std::cout<<" alias : " << alias << std::endl;
            ret = true;
            break;
        }
    }
    working_mtx_.Unlock();
    //Find post id, the config table key has the id, the state is the filepath
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->RetrieveConfigValue(alias, post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::IsRootDirectory(const std::string& post_id) {
    bool ret = false;
    manifest_mtx_.Lock();
    ret = manifest_.config_table()->IsValueInManifest(post_id);
    manifest_mtx_.Unlock();
    return ret;
}

bool FileManager::ConstructFolderpath(const std::string& folder_post_id, 
                                      std::string& path_out) {
    bool ret = false;
    std::string post_id = folder_post_id;
    for(;;) {
        Folder folder;
        std::cout<<" Get Folder entry by post id for : " << post_id << std::endl;
        std::cout<<" folder post id : " << folder_post_id << std::endl;
        if(GetFolderEntryByPostId(post_id, folder)) {
            if(!IsRootDirectory(post_id)) { 
                if(folder.has_alias())
                    path_out = "/" + folder.local_alias() + path_out;
                else
                    path_out = "/" + folder.foldername() + path_out;
            }
            else {
                if(folder.has_alias())
                    path_out = folder.local_alias() + path_out;
                else
                    path_out = folder.foldername() + path_out;
                ret = true;
                break;
            }
            std::cout<<" path out : "<< path_out << std::endl;
            post_id = folder.parent_post_id();
        }
        else  {
            std::cout<<" COULDNT FIND FOLDER POST FOR : " << folder_post_id << std::endl;
            break;
        }
    }
    return ret;
}




}//namespace
